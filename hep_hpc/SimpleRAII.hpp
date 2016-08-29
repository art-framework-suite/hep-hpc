#ifndef HDFSTUDY_SIMPLERAII_HPP
#define HDFSTUDY_SIMPLERAII_HPP
////////////////////////////////////////////////////////////////////////
// hep_hpc::SimpleRAII
//
// Simple generic RAII class template.
//
// SimpleRAII has one template argument RESOURCE_HANDLE, that describes
// the type of a handle to the resource to be managed.
//
// The single constructor template takes:
//
// 1. A setup function which will return an instance of a type that may
//    be stored as a RESOURCE_HANDLE.
//
// 2. A teardown function with void return type taking a (possibly
//    movable) RESOURCE_HANDLE as argument.
//
// 3. Optionally, any number of arguments to forward to the setup
//    function when invoked.
//
////////////////////////////////////
// NOTES
//
// * At construction time, the setup function is invoked with the
//   provided arguments (if any) and the returned RESOURCE_HANDLE is
//   stored.
//
// * At destruction time, the teardown function is invoked with the
//   stored RESOURCE_HANDLE as its only argument.
//
// * A specialization is provided for the (rare) case of required paired
//   setup / teardown operations which do not provide a handle to a
//   resource (an example would be MPI initialization / finalization).
//
// * In order to help assure the exception safety of classes using this
//   template, efforts have been made to ensure the correctness of
//   noexcept declarations. Therefore the code below is not pretty.
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/is_nothrow_swappable_all.hpp"

#include <functional>
#include <type_traits>
#include <utility>

namespace hep_hpc {
  template <typename RESOURCE_HANDLE>
  class SimpleRAII;
  // Specialization for no handle.
  template <>
  class SimpleRAII<void>;
}

// Class template definition.
template <typename RESOURCE_HANDLE>
class hep_hpc::SimpleRAII {
public:
  SimpleRAII() = default;

  template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
  SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args)
  noexcept(noexcept(setup(std::forward<ARGS>(args)...)) &&
           noexcept(TEARDOWN_FUNC(teardown)));

  template <typename T>
  friend
  void swap(SimpleRAII<T> & left, SimpleRAII<T> & right)
noexcept(is_nothrow_swappable_all<decltype(left.resourceHandle_),
         decltype(left.teardown_)>::value)
    {
      using std::swap;
      swap(left.resourceHandle_, right.resourceHandle_);
      swap(left.teardown_, right.teardown_);
    }

  SimpleRAII(SimpleRAII<RESOURCE_HANDLE> && other)
 noexcept(std::is_nothrow_constructible<SimpleRAII<RESOURCE_HANDLE>>::value &&
          noexcept(swap(other, other))) = default;
  SimpleRAII<RESOURCE_HANDLE> & operator = (SimpleRAII<RESOURCE_HANDLE> && other)
          noexcept(is_nothrow_swappable_all<decltype(other.resourceHandle_),
                   decltype(other.teardown_)>::value) = default;
  SimpleRAII(SimpleRAII<RESOURCE_HANDLE> const &) = delete;
  SimpleRAII<RESOURCE_HANDLE> &
  operator = (SimpleRAII<RESOURCE_HANDLE> const &) = delete;

  RESOURCE_HANDLE const & operator * () const noexcept;
  RESOURCE_HANDLE & operator * () noexcept;

  ~SimpleRAII() noexcept;
private:
  RESOURCE_HANDLE resourceHandle_;
  std::function<void(RESOURCE_HANDLE &&)> teardown_;
};

// Class template specialization definition.
template <>
class hep_hpc::SimpleRAII<void> {
public:
  SimpleRAII() noexcept = default;

  template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
  SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args)
noexcept(noexcept(setup(std::forward<ARGS>(args)...)) &&
         noexcept(TEARDOWN_FUNC(teardown)));

  friend void swap(SimpleRAII<void> & left, SimpleRAII<void> & right)
noexcept(is_nothrow_swappable_all<std::function<void()>>::value)
    {
      using std::swap;
      swap(left.teardown_, right.teardown_);
    }

  SimpleRAII(SimpleRAII<void> && other)
  noexcept(std::is_nothrow_constructible<SimpleRAII<void>>::value &&
           noexcept(swap(other, other))) = default;
  SimpleRAII<void> & operator = (SimpleRAII<void> && other)
 noexcept(is_nothrow_swappable_all<std::function<void()>>::value) = default;
  SimpleRAII(SimpleRAII<void> const &) = delete;
  SimpleRAII<void> & operator = (SimpleRAII<void> const &) = delete;

  ~SimpleRAII() noexcept;
private:
  std::function<void()> teardown_;
};

////////////////////////////////////////////////////////////////////////
// Implementation of class template.
template <typename RESOURCE_HANDLE>
template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
inline
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args)
  noexcept(noexcept(setup(std::forward<ARGS>(args)...)) &&
           noexcept(TEARDOWN_FUNC(teardown)))
  :
  resourceHandle_(setup(std::forward<ARGS>(args)...)),
  teardown_(std::move(teardown))
{
}

template <typename RESOURCE_HANDLE>
inline
auto
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
operator * () const noexcept
-> RESOURCE_HANDLE const &
{
  return resourceHandle_;
}

template <typename RESOURCE_HANDLE>
inline
auto
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
operator * () noexcept
-> RESOURCE_HANDLE &
{
  return resourceHandle_;
}

template <typename RESOURCE_HANDLE>
inline
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
~SimpleRAII() noexcept
{
  if (teardown_) {
    teardown_(std::move(resourceHandle_));
  }
}
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Implementation of specialization.
template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
inline
hep_hpc::SimpleRAII<void>::
SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args)
noexcept(noexcept(setup(std::forward<ARGS>(args)...)) &&
         noexcept(TEARDOWN_FUNC(teardown)))
  :
  teardown_(teardown)
{
  setup(std::forward<ARGS>(args)...);
}

inline
hep_hpc::SimpleRAII<void>::
~SimpleRAII() noexcept
{
  if (teardown_) {
    teardown_();
  }
}
////////////////////////////////////////////////////////////////////////

#endif /* HDFSTUDY_SIMPLERAII_HPP */
