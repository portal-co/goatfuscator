#include "Util.h"
#include <cstdint>
#include <llvm-16/llvm/IR/BasicBlock.h>
#include <llvm-16/llvm/IR/Instruction.h>
#include <llvm-16/llvm/Support/Casting.h>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
using namespace llvm;
namespace {};
uint32_t emit(Value *v, std::string &y, BasicBlock *tbb) {
  uint32_t x = xr();
  Instruction *i = dyn_cast<Instruction>(v);
  if (i == nullptr || i->getParent() != tbb) {
    y += "c.push_back([=](v &v){v.p(";
    y += std::to_string(x);
    y += ",";
    y += v->getName();
    y += ");v.r();})";
    return x;
  }
  std::vector<uint32_t> d;
  for (auto j : i->operand_values()) {
    // if (dyn_cast<Instruction *>(k) != nullptr) {
    d.push_back(emit(j, y, tbb));
    // }
  }
  if (i->isBinaryOp()) {
    std::string a = "<uint64_t, ";
    a += std::to_string(d[0]);
    a += ",uint64_t,";
    a += std::to_string(d[1]);
    a += ",uint64_t,";
    a += std::to_string(x);
    a += ">";
    std::string b;
    switch (i->getOpcode()) {
    case Instruction::BinaryOps::Add:
      b = "add";
      break;
    case Instruction::BinaryOps::Sub:
      b = "sub";
      break;
    case Instruction::BinaryOps::Mul:
      b = "mul";
      break;
    case Instruction::BinaryOps::UDiv:
      b = "div";
      break;
    case Instruction::BinaryOps::Xor:
      b = "exor";
      break;
    case Instruction::BinaryOps::Or:
      b = "eor";
      break;
    case Instruction::BinaryOps::And:
      b = "eand";
      break;
    };
    y += "c.push_back(&";
    y += b;
    y += a;
    y += ");";
  };
  return x;
}