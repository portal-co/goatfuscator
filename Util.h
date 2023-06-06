#include "llvm/IR/Function.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/PatternMatch.h"
#include <llvm-16/llvm/IR/BasicBlock.h>
#include <llvm-16/llvm/IR/InstrTypes.h>
#include <llvm-16/llvm/IR/Instruction.h>
#include <llvm-16/llvm/IR/Module.h>
#include <llvm-16/llvm/IR/PassManager.h>
#include <llvm-16/llvm/Support/GenericDomTree.h>
#include "llvm/Linker/Linker.h"


#include <memory>
#include <tuple>
#include <variant>
#include <vector>
#include <sys/stat.h>

void fixStack(llvm::Function *f);

const uint32_t fnvPrime = 19260817;
const uint32_t fnvBasis = 0x114514;
uint32_t fnvHash(const uint32_t data, uint32_t b);
llvm::InlineAsm *generateGarbage(llvm::Function *f);
uint32_t randPrime(uint32_t min, uint32_t max);
uint64_t modinv(uint64_t a);
void addBB2Func(llvm::PassManager<llvm::Function> &);
void addFlattening(llvm::PassManager<llvm::Function> &);
void addConnect(llvm::PassManager<llvm::Function> &);
void addObfCon(llvm::PassManager<llvm::Function> &);
void addMerge(llvm::PassManager<llvm::Module> &);

void runObfCon(llvm::Function &F);
void link(llvm::Module &m, std::string code);
inline bool exists (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}
// struct vmCode {
//   std::variant<unsigned, llvm::Function *, llvm::Value *> value;
//   template <typename X> vmCode(X x) : value(x) {}
// };
// struct VMCodePass : public llvm::AnalysisInfoMixin<VMCodePass> {
//   // A map that for every basic block holds the VM code for it
//   struct CodeInfo {
//     std::vector<vmCode> code;
//     unsigned maxSize;
//     void render(llvm::Function *&);
//   };
//   using Result = llvm::MapVector<llvm::BasicBlock const *, CodeInfo>;
//   Result run(llvm::Function &F, llvm::FunctionAnalysisManager &);
//   Result buildVM(llvm::Function &F,
//                  llvm::DomTreeNodeBase<llvm::BasicBlock> *CFGRoot);

// private:
//   // A special type used by analysis passes to provide an address that
//   // identifies that particular analysis pass type.
//   static llvm::AnalysisKey Key;
//   friend struct llvm::AnalysisInfoMixin<VMCodePass>;
// };

template <typename X, typename Y> auto randItem(X x, Y y) {
  auto Iter = x.begin();
  std::uniform_int_distribution<> Dist(0, x.size() - 1);
  std::advance(Iter, Dist(y));
  return *Iter;
}
namespace llvm {
//===----------------------------------------------------------------------===//
// Matcher for any binary operator.
//
template <typename LHS_t, typename RHS_t, bool Commutable = false>
struct AnyBinaryOp_xmatch {
  LHS_t L;
  RHS_t R;
  Instruction::BinaryOps &Ops;

  // The evaluation order is always stable, regardless of Commutability.
  // The LHS is always matched first.
  AnyBinaryOp_xmatch(const LHS_t &LHS, const RHS_t &RHS,
                     Instruction::BinaryOps &o)
      : L(LHS), R(RHS), Ops(o) {}

  template <typename OpTy> bool match(OpTy *V) {
    if (auto *I = dyn_cast<BinaryOperator>(V)) {

      auto b = (L.match(I->getOperand(0)) && R.match(I->getOperand(1))) ||
               (Commutable && L.match(I->getOperand(1)) &&
                R.match(I->getOperand(0)));
      if (b)
        Ops = I.getOpcode();
      return b;
    }
    return false;
  }
};

template <typename LHS, typename RHS>
inline AnyBinaryOp_xmatch<LHS, RHS> m_BinOp(const LHS &L, const RHS &R,
                                            Instruction::BinaryOps &o) {
  return AnyBinaryOp_xmatch<LHS, RHS>(L, R, o);
}

//===----------------------------------------------------------------------===//
// Matcher for any unary operator.
// TODO fuse unary, binary matcher into n-ary matcher
//
template <typename OP_t> struct AnyUnaryOp_xmatch {
  OP_t X;
  Instruction::UnaryOps &Ops;

  AnyUnaryOp_xmatch(const OP_t &X, Instruction::UnaryOps &Ops)
      : X(X), Ops(Ops) {}

  template <typename OpTy> bool match(OpTy *V) {
    if (auto *I = dyn_cast<UnaryOperator>(V))
      if (X.match(I->getOperand(0))) {
        Ops = I->getOpcode();
        return true;
      };
    return false;
  }
};

template <typename OP_t>
inline AnyUnaryOp_xmatch<OP_t> m_UnOp(const OP_t &X,
                                      Instruction::UnaryOps &Ops) {
  return AnyUnaryOp_xmatch<OP_t>(X, Ops);
}
} // namespace llvm