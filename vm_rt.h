#include <any>
#include <cstdint>
#include <deque>
#include <map>
#include <vector>
struct v {
  std::vector<void (*)(v &)> c;
  size_t s;
  std::map<uint32_t, std::deque<std::any>> k;
  inline void r() {
    s++;
    c[s - 1](*this);
  }
  inline std::any p(uint32_t x) {
    auto &l = k[x];
    auto b = l.back();
    l.pop_back();
    return b;
  }
  template <typename X> void o(uint32_t x, X &y) { y = std::any_cast<X>(p(x)); }
  template <typename X> inline void p(uint32_t x, X a) { k[x].push_back(a); }
};
#define A2(n)                                                                  \
  template <typename I, uint32_t IJ, typename I2, uint32_t I2J, typename O,    \
            uint32_t OJ>                                                       \
  void n(v &x)
#define A1(n)                                                                  \
  template <typename I, uint32_t IJ, typename O, uint32_t OJ> void n(v &x)

A2(add) {
  I i;
  x.o(IJ, i);
  I2 i2;
  x.o(I2J, i2);
  O o = i + i2;
  x.p(OJ, o);
  x.r();
};
A2(sub) {
  I i;
  x.o(IJ, i);
  I2 i2;
  x.o(I2J, i2);
  O o = i - i2;
  x.p(OJ, o);
  x.r();
};
A2(mul) {
  I i;
  x.o(IJ, i);
  I2 i2;
  x.o(I2J, i2);
  O o = i * i2;
  x.p(OJ, o);
  x.r();
};
A2(div) {
  I i;
  x.o(IJ, i);
  I2 i2;
  x.o(I2J, i2);
  O o = i / i2;
  x.p(OJ, o);
  x.r();
};
A2(exor) {
  I i;
  x.o(IJ, i);
  I2 i2;
  x.o(I2J, i2);
  O o = i ^ i2;
  x.p(OJ, o);
  x.r();
};
A2(eor) {
  I i;
  x.o(IJ, i);
  I2 i2;
  x.o(I2J, i2);
  O o = i | i2;
  x.p(OJ, o);
  x.r();
};
A2(eand) {
  I i;
  x.o(IJ, i);
  I2 i2;
  x.o(I2J, i2);
  O o = i & i2;
  x.p(OJ, o);
  x.r();
};
A1(no) {
  I i;
  x.o(IJ, i);
  O o = !i;
  x.p(OJ, o);
  x.r();
};
A1(eno) {
  I i;
  x.o(IJ, i);
  O o = ~i;
  x.p(OJ, o);
  x.r();
};