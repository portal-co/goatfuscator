#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BlockFrequencyInfoImpl.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/BlockFrequency.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm-17/llvm/IR/IRBuilder.h>
#include <map>
#include <memory>
#include <vector>

using namespace llvm;
bool definedInCaller(BasicBlock *BB, Value *V) {
  if (isa<Argument>(V))
    return true;
  if (Instruction *I = dyn_cast<Instruction>(V))
    if (I->getParent() != BB)
      return true;
  return false;
}
void findInputsOutputs(BasicBlock *BB, SmallPtrSet<Value *, 32> &Inputs) {
  //   for (BasicBlock *BB : Blocks) {
  // If a used value is defined outside the region, it's an input.  If an
  // instruction is used outside the region, it's an output.
  for (Instruction &II : *BB) {
    for (auto &OI : II.operands()) {
      Value *V = OI;
      if (definedInCaller(BB, V))
        Inputs.insert(V);
    }

    //   for (User *U : II.users())
    //     if (!definedInRegion(Blocks, U)) {
    //       Outputs.insert(&II);
    //       break;
    //     }
  }
  //   }
}
std::vector<Value *> io(BasicBlock *BB) {
  SmallPtrSet<Value *, 32> i;
  findInputsOutputs(BB, i);
  std::vector<Value *> v;
  for (auto j : i)
    v.push_back(j);
  return v;
}
struct Bundle {
  llvm::Function *func;
  std::vector<Value *> args;
};
template <typename R>
Bundle constructFunction(BasicBlock *header, Function *oldFunction, Module *M,
                         R redo) {
  std::vector<Type *> ParamTy;
  //   std::vector<Type *> AggParamTy;
  //   ValueSet StructValues;
  const DataLayout &DL = M->getDataLayout();
  std::vector<Value *> inputs = io(header);

  // Add the types of the input values to the function's argument list
  for (Value *value : inputs) {
    // LLVM_DEBUG(dbgs() << "value used in func: " << *value << "\n");
    // if (AggregateArgs && !ExcludeArgsFromAggregate.contains(value)) {
    //   AggParamTy.push_back(value->getType());
    //   StructValues.insert(value);
    // } else
    ParamTy.push_back(value->getType());
  }
  FunctionType *funcType =
      FunctionType::get(oldFunction->getReturnType(), ParamTy, false);

  std::string SuffixToUse = header->getName().str();
  // Create the new function
  Function *newFunction = Function::Create(
      funcType, GlobalValue::InternalLinkage, oldFunction->getAddressSpace(),
      oldFunction->getName() + "." + SuffixToUse, M);
  header->removeFromParent();
  newFunction->insert(newFunction->end(), header);
  Function::arg_iterator ScalarAI = newFunction->arg_begin();
  std::map<Value *, Value *> rew;
  // Rewrite all users of the inputs in the extracted region to use the
  // arguments (or appropriate addressing into struct) instead.
  for (unsigned i = 0, e = inputs.size(), aggIdx = 0; i != e; ++i) {
    Value *RewriteVal;
    // if (AggregateArgs && StructValues.contains(inputs[i])) {
    //   Value *Idx[2];
    //   Idx[0] =
    //   Constant::getNullValue(Type::getInt32Ty(header->getContext())); Idx[1]
    //   = ConstantInt::get(Type::getInt32Ty(header->getContext()), aggIdx);
    //   Instruction *TI = newFunction->begin()->getTerminator();
    //   GetElementPtrInst *GEP = GetElementPtrInst::Create(
    //       StructTy, &*AggAI, Idx, "gep_" + inputs[i]->getName(), TI);
    //   RewriteVal = new LoadInst(StructTy->getElementType(aggIdx), GEP,
    //                             "loadgep_" + inputs[i]->getName(), TI);
    //   ++aggIdx;
    // } else
    RewriteVal = &*ScalarAI++;
    rew[inputs[i]] = RewriteVal;

    std::vector<User *> Users(inputs[i]->user_begin(), inputs[i]->user_end());
    for (User *use : Users)
      if (Instruction *inst = dyn_cast<Instruction>(use))
        if (inst->getParent() == header)
          inst->replaceUsesOfWith(inputs[i], RewriteVal);
  }
  //   std::vector<User *> Users(header->user_begin(), header->user_end());
  //   for (auto &U : Users)
  //     // The BasicBlock which contains the branch is not in the region
  //     // modify the branch target to a new block
  //     if (Instruction *I = dyn_cast<Instruction>(U))
  //       if (I->isTerminator() && I->getFunction() == oldFunction &&
  //           I->getParent() != BB)
  //         I->replaceUsesOfWith(header, );

  auto t = header->getTerminator();
  for (int sb = 0; sb < t->getNumSuccessors(); sb++) {
    auto s = t->getSuccessor(sb);
    Bundle sr = redo(s);
    for (auto &r : sr.args) {
      while (rew.contains(r)) {
        r = rew[r];
      }
    }
    auto b = BasicBlock::Create(M->getContext());
    t->setSuccessor(sb, b);
    IRBuilder i(b);
    auto c = i.CreateCall(sr.func, sr.args);
    i.CreateRet(c);
  }

  return Bundle{
      .func = newFunction,
      .args = inputs,
  };
}
struct ConstructState {
  std::map<BasicBlock *, Bundle> bundles;
};
Bundle constructFunction(BasicBlock *header, Function *oldFunction, Module *M,
                         std::shared_ptr<ConstructState> state =
                             std::make_shared<ConstructState>()) {
  return state->bundles[header] = constructFunction(
             header, oldFunction, M, [=](BasicBlock *b) -> Bundle {
               if (state->bundles.count(b))
                 return state->bundles[b];
               return constructFunction(b, oldFunction, M, state);
             });
}