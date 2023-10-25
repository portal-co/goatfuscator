#include <any>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <vector>
namespace OBF_NS {
using vm = uint64_t (*)(void *pc, void* stack);
inline uint64_t call(void *pc,void* s) {
  vm *v = (vm *)pc;
  vm a = *v;
  v++;
  return a((void *)v, s);
}
} // namespace OBF_NS