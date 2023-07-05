#pragma once

// For std::decay
#include <type_traits>

template<typename,typename>
struct TransientFunction; // intentionally not defined

template<typename R,typename Alloc, typename ...Args>
struct TransientFunction<R(Args...),Alloc>
{
  using Dispatcher = R(*)(void*, Args...);
  
  Dispatcher m_Dispatcher; // A pointer to the static function that will call the 
                           // wrapped invokable object
  typename Alloc::Ptr m_Target;          // A pointer to the invokable object

  // Dispatch() is instantiated by the TransientFunction constructor,
  // which will store a pointer to the function in m_Dispatcher.
  template<typename S>
  static R Dispatch(void* target, Args... args)
  {
    return (*(S*)target)(args...);
  }
  
  template<typename T>
  TransientFunction(T& target, Alloc alloc = Alloc::global)
    : m_Dispatcher(&Dispatch<typename std::decay<T>::type>)
    , m_Target(&target, alloc)
  {
  }

//   // Specialize for reference-to-function, to ensure that a valid pointer is 
//   // stored.
//   using TargetFunctionRef = R(Args...);
//   TransientFunction(TargetFunctionRef target)
//     : m_Dispatcher(Dispatch<TargetFunctionRef>)
//   {
//     static_assert(sizeof(void*) == sizeof target, 
//     "It will not be possible to pass functions by reference on this platform. "
//     "Please use explicit function pointers i.e. foo(target) -> foo(&target)");
//     m_Target = (void*)target;
//   }

  R operator()(Args... args) const
  {
    return m_Dispatcher(m_Target, args...);
  }
};