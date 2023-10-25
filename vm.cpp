#include "Util.h"
#include <cstdint>
#include <exception>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
using namespace llvm;
// struct vm_key{
//   unsigned id;
//   Value *target;
//   static inline vm_key get(llvm::Instruction *i){
//     if(isa<CallInst>(i)){
//       return vm_key{i->getOpcode(),cast<CallInst>(i)->getCalledOperand()};
//     }
//     return vm_key{i->getOpcode(), nullptr};
//   }
//   bool operator==(vm_key const&) const = default;
// };
// namespace std{
//   template<>struct hash<vm_key>{
//     size_t operator()(const vm_key &v) const{
//       return hash<unsigned>{}(v.id) ^ (size_t)v.target;
//     }
//   };
// };
// namespace {
// struct non_vmable : std::exception {};
// }; // namespace
// uint32_t emit(Value *v, std::string &y, BasicBlock *tbb) {
//   uint32_t x = xr();
//   Instruction *i = dyn_cast<Instruction>(v);
//   if (i == nullptr || i->getParent() != tbb) {
//     throw non_vmable();
//   }
//   std::vector<uint32_t> d;
//   for (auto j : i->operand_values()) {
//     // if (dyn_cast<Instruction *>(k) != nullptr) {
//     d.push_back(emit(j, y, tbb));
//     // }
//   }
//   if (i->isBinaryOp()) {
//     std::string a = "<uint64_t, ";
//     a += std::to_string(d[0]);
//     a += ",uint64_t,";
//     a += std::to_string(d[1]);
//     a += ",uint64_t,";
//     a += std::to_string(x);
//     a += ">";
//     std::string b;
//     switch (i->getOpcode()) {
//     case Instruction::BinaryOps::Add:
//       b = "add";
//       break;
//     case Instruction::BinaryOps::Sub:
//       b = "sub";
//       break;
//     case Instruction::BinaryOps::Mul:
//       b = "mul";
//       break;
//     case Instruction::BinaryOps::UDiv:
//       b = "div";
//       break;
//     case Instruction::BinaryOps::Xor:
//       b = "exor";
//       break;
//     case Instruction::BinaryOps::Or:
//       b = "eor";
//       break;
//     case Instruction::BinaryOps::And:
//       b = "eand";
//       break;
//     default:
//       throw non_vmable();
//     };
//     y += "c.push_back(&";
//     y += b;
//     y += a;
//     y += ");";
//   };
//   return x;
// }