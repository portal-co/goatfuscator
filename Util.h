#include "llvm/IR/Function.h"
#include "llvm/IR/InlineAsm.h"
#include <llvm-16/llvm/IR/PassManager.h>

void fixStack(llvm::Function *f);

const uint32_t fnvPrime = 19260817;
const uint32_t fnvBasis = 0x114514;
uint32_t fnvHash(const uint32_t data, uint32_t b);
llvm::InlineAsm *generateGarbage(llvm::Function *f);
uint32_t randPrime(uint32_t min, uint32_t max);
uint64_t modinv(uint64_t a);
void addBB2Func(llvm::PassManager<llvm::Function>&);
void addFlattening(llvm::PassManager<llvm::Function>&);
void addConnect(llvm::PassManager<llvm::Function>&);
void addMerge(llvm::PassManager<llvm::Module>&);