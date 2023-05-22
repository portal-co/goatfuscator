#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include "Util.h"
#include <algorithm>
#include <llvm/IR/PassManager.h>
#include <vector>
#include <list>

using namespace llvm;

// Stats

namespace {
struct BB2Func : public PassInfoMixin<BB2Func> {
  static char ID;

  BB2Func() {}

  bool runOnFunction(Function &F);
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM){
    runOnFunction(F);
    return PreservedAnalyses::none();
  }
};
} // namespace

char BB2Func::ID = 0;
// static RegisterPass<BB2Func> X("bb2func", "Split & extract basic blocks to functions");
// Pass *createBB2FuncPass() { return new BB2Func(); }

void addBB2Func(llvm::PassManager<llvm::Function> & f){
  f.addPass(BB2Func());
}

bool BB2Func::runOnFunction(Function &F) {
  bool modified = false;
  if(F.getEntryBlock().getName() == "newFuncRoot")
    return modified;
  CodeExtractorAnalysisCache cache(F);
  std::list<BasicBlock *> bblist;
  for(auto i = F.begin(); i != F.end(); ++i){
    BasicBlock *BB = &*i;
    if(BB->size() > 4){
      std::vector<BasicBlock *> blocks;
      blocks.push_back(BB);
      CodeExtractor CE(blocks);
      if(CE.isEligible()){
        bblist.push_back(BB);
      }
    }
  }
  
  size_t sizeLimit = 16;
  if(bblist.size() > sizeLimit){
    bblist.sort([](const BasicBlock *a, const BasicBlock *b){
              return a->size() > b->size();});
    auto it = bblist.begin();
    std::advance(it, sizeLimit);
    bblist.erase(it, bblist.end());
  }

  for(auto it = bblist.begin(); it != bblist.end(); it++){
    BasicBlock *BB = *it;
    BasicBlock::iterator itb = BB->getFirstInsertionPt();
    size_t bbSize = std::distance(itb, BB->end());
    if(bbSize >= 4){
      std::advance(itb, bbSize/2 > 8 ? 8 : bbSize/2);
      bblist.push_back(BB->splitBasicBlock(itb));
    }
  }

  for(BasicBlock *BB: bblist){
    std::vector<BasicBlock *> blocks;
    blocks.push_back(BB);
    CodeExtractor CE(blocks);
    assert(CE.isEligible());
    Function *F = CE.extractCodeRegion(cache);
    F->addFnAttr(Attribute::NoInline);
    modified = true;
  }
  return modified;
}