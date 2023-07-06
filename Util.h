#include "llvm/IR/Function.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Linker/Linker.h"
#include <cstdint>
#include <cstdlib>
#include <llvm-16/llvm/ADT/ArrayRef.h>
#include <llvm-16/llvm/IR/BasicBlock.h>
#include <llvm-16/llvm/IR/DerivedTypes.h>
#include <llvm-16/llvm/IR/InstrTypes.h>
#include <llvm-16/llvm/IR/Instruction.h>
#include <llvm-16/llvm/IR/Module.h>
#include <llvm-16/llvm/IR/PassManager.h>
#include <llvm-16/llvm/Passes/PassBuilder.h>
#include <llvm-16/llvm/Support/GenericDomTree.h>

#include <map>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <tuple>
#include <variant>
#include <vector>
#include <random>
#include <sstream>
#include <fstream>

void fixStack(llvm::Function *f);

const uint32_t fnvPrime = 19260817;
const uint32_t fnvBasis = 0x114514;
uint32_t fnvHash(const uint32_t data, uint32_t b);
llvm::InlineAsm *generateGarbage(llvm::Function *f);
llvm::InlineAsm *generateNull(llvm::Function *f);
uint32_t randPrime(uint32_t min, uint32_t max);
uint64_t modinv(uint64_t a);
uint32_t xr();
void addBB2Func(llvm::PassManager<llvm::Function> &);
void addFlattening(llvm::PassManager<llvm::Function> &);
void addConnect(llvm::PassManager<llvm::Function> &);
void addObfCon(llvm::PassManager<llvm::Function> &);
void addMerge(llvm::PassManager<llvm::Module> &);
void addRIV(llvm::PassBuilder &);

void runObfCon(llvm::Function &F);
void link(llvm::Module &m, std::string code);
inline bool exists(const std::string &name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}
inline std::string gen_namespace(std::string x) {
  std::random_device rd;
  std::string n = "O_";
  n += std::to_string(rd());
  std::string r = "namespace ";
  r += n;
  r += "{namespace{";
  r += x;
  r += "}}";
  r += "using namespace ";
  r += n;
  r += ";";
  return r;
}
inline std::string read(std::string x) {
  std::ifstream t(x);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}
inline std::string goatfDir(std::string x) {
  return std::string(getenv("GOATF_DIR")) + "/" + x;
}

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