#pragma once
#include "Util.h"
#include "regalloc2.h"
#include "llvm/IR/InstVisitor.h"
#include <cstdint>
#include <llvm-16/llvm/IR/Type.h>
#include <llvm-16/llvm/IR/Value.h>
#include <llvm-17/llvm/IR/Instruction.h>
#include <llvm-17/llvm/IR/Instructions.h>
#include <llvm-17/llvm/Support/Casting.h>
#include <memory>
#include <string>
#include <tuple>
#include <variant>
#include <vector>
struct VMInstr {
  std::string id;
  std::vector<std::tuple<RegOrImm, llvm::Type *>> all;
  llvm::Type *type;
  std::vector<llvm::Type *> typarams;
  uint64_t retreg;
  static std::vector<std::tuple<RegOrImm, llvm::Type *>>
  find_all(RegAlloc const &map, llvm::Instruction *i) {
    std::vector<std::tuple<RegOrImm, llvm::Type *>> j;
    for (auto &o : i->operands())
      j.push_back({get_reg(map, o.get()), o->getType()});
    return j;
  }
  VMInstr(RegAlloc const &map, std::string id, llvm::Instruction *i)
      : id(id), type(i->getType()), retreg(map.map.at(i)),
        all(find_all(map, i)) {
    if (llvm::isa<llvm::AllocaInst>(i)) {
      typarams.push_back(((llvm::AllocaInst *)i)->getAllocatedType());
    };
    // vi.id="alloca";
    // vi.type = i.getType();
    // vi.retreg = alloc.map[&i];
    // vi.all = {{get_reg(alloc, i.getOperand(0)),i.getOperand(0)->getType()}};
  }
};
struct VMVisitor : llvm::InstVisitor<VMVisitor> {
  RegAlloc alloc;
  std::shared_ptr<std::vector<VMInstr>> instrs;
#define HANDLE_INST(NUM, OPCODE, CLASS)                                        \
  void visit##OPCODE(llvm::CLASS &i) {                                         \
    VMInstr vi(alloc, #OPCODE, &i);                                            \
    instrs->push_back(vi);                                                     \
  };
#include "llvm/IR/Instruction.def"
};
