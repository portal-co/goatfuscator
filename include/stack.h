#include <alloca.h>
template <typename T> struct stack {
  virtual ~stack();
  virtual T pop();
  virtual void push(T v);
};
template <typename T, size_t N> struct stack_impl : stack<T> {
  T vals[N];
  size_t p = 0;
  T pop() override {
    T a = vals[p];
    p--;
    return a;
  }
  void push(T v) override {
    p++;
    vals[p] = v;
  }
};
template <typename T, typename U> [[always_inline]] void pushx(T &t, U u) {
  U *v = (U *)alloca(sizeof(U));
  v->U();
  *v = u;
  t.push((void *)v);
}
template <typename T, typename U> T popx(U &t) {
  T *u = (T *)t.pop();
  T tt = *u;
  u->~T();
  return tt;
}
template <typename T, typename U> void popx(U &t, T &uu) {
  T *u = (T *)t.pop();
  uu = *u;
  u->~T();
}
