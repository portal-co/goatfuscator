#include "llvm/IR/InstIterator.h"
#include <cstdint>
#include <llvm-17/llvm/IR/Function.h>
#include <llvm-17/llvm/IR/Value.h>
#include <map>
#include <set>
#include <variant>
#include <vector>
struct RegAlloc{
std::map<llvm::Value *, uint64_t> map;
uint64_t len;
};
inline RegAlloc ra2(llvm::Function &F) {
  std::vector<llvm::Instruction *> worklist;
  // or better yet, SmallPtrSet<Instruction*, 64> worklist;

  for (llvm::inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    worklist.push_back(&*I);

  std::map<llvm::Value *, uint64_t> r;
  uint64_t n = 0;
  for (auto i : worklist) {
    if (!r.contains(i)) {
      r[i] = n;
      n++;
    };
    if (i->hasOneUse() && (*i->use_begin())->getType() == i->getType()) {
      r[*i->use_begin()] = r[i];
    }
  };
  return RegAlloc{
    .map = r,
    .len = n,
  };
};
using RegOrImm = std::variant<uint64_t, llvm::Value *>;
inline RegOrImm get_reg(RegAlloc const &r, llvm::Value *v){
    if(r.map.contains(v))return r.map.at(v);
    return v;
}