#ifndef hep_hpc_SimpleRAII_hpp
#define hep_hpc_SimpleRAII_hpp
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
  // SimpleRAII template.
  template <typename RESOURCE_HANDLE>
  class SimpleRAII;

  // Specialization for no resource handle.
  template <>
  class SimpleRAII<void>;

  // Free swap functions.
  template <typename RH>
  void swap(SimpleRAII<RH> & left, SimpleRAII<RH> & right);

  void swap(SimpleRAII<void> & left, SimpleRAII<void> & right);
}

// Class template definition.
template <typename RESOURCE_HANDLE>
class hep_hpc::SimpleRAII {
public:
  // Default constructor.
  SimpleRAII() = default;

  // Constructor with arguments.
  template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
  SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args);

  // Free swap function.
  template <typename RH>
  friend void swap(SimpleRAII<RH> & left, SimpleRAII<RH> & right);

  // Move constructor.
  SimpleRAII(SimpleRAII<RESOURCE_HANDLE> && other);

  // Move assignment.
  SimpleRAII<RESOURCE_HANDLE> &
  operator = (SimpleRAII<RESOURCE_HANDLE> && other);

  // No copy constructor.
  SimpleRAII(SimpleRAII<RESOURCE_HANDLE> const &) = delete;

  // No move constructor.
  SimpleRAII<RESOURCE_HANDLE> &
  operator = (SimpleRAII<RESOURCE_HANDLE> const &) = delete;

  // Dereference operators.
  RESOURCE_HANDLE const & operator * () const;
  RESOURCE_HANDLE & operator * ();

  // Access teardown function.
  std::function<void(RESOURCE_HANDLE) &&> teardownFunc() const;

  // Release (teardown function will be neutralized).
  RESOURCE_HANDLE release();

  // Destructor.
  ~SimpleRAII();

private:
  RESOURCE_HANDLE resourceHandle_;
  std::function<void(RESOURCE_HANDLE &&)> teardown_;
};

// Class template specialization definition.
template <>
class hep_hpc::SimpleRAII<void> {
public:
  // Default constructor.
  SimpleRAII() noexcept = default;

  // Constructor with arguments.
  template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
  SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args);

  // Free swap function
  friend
  void swap(SimpleRAII<void> & left, SimpleRAII<void> & right);

  // Move constructor.
  SimpleRAII(SimpleRAII<void> && other);

  // Move assignment.
  SimpleRAII<void> &
  operator = (SimpleRAII<void> && other);

  // No copy constructor.
  SimpleRAII(SimpleRAII<void> const &) = delete;

  // No copy assignment.
  SimpleRAII<void> &
  operator = (SimpleRAII<void> const &) = delete;

  // Access teardown function.
  std::function<void()> teardownFunc() const;

  // Release (teardown function will be neutralized).
  void release();

  // Destructor.
  ~SimpleRAII();

private:
  std::function<void()> teardown_;
};

////////////////////////////////////////////////////////////////////////
// Implementation of class template.

// Constructor with arguments.
template <typename RESOURCE_HANDLE>
template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
inline
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args)
  :
  resourceHandle_(setup(std::forward<ARGS>(args)...)),
  teardown_(teardown)
{
}

// Move constructor.
template <typename RESOURCE_HANDLE>
inline
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
SimpleRAII(SimpleRAII<RESOURCE_HANDLE> && other)
  :
  resourceHandle_(other.resourceHandle_),
  teardown_(other.teardown_)
{
  (void) other.release();
}

// Move assignment.
template <typename RESOURCE_HANDLE>
inline
hep_hpc::SimpleRAII<RESOURCE_HANDLE> &
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
operator = (SimpleRAII<RESOURCE_HANDLE> && other) {
  swap(*this, other);
  return *this;
}

// Dereference operators.
template <typename RESOURCE_HANDLE>
inline
RESOURCE_HANDLE const &
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
operator * () const
{
  return resourceHandle_;
}

template <typename RESOURCE_HANDLE>
inline
RESOURCE_HANDLE &
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
operator * ()
{
  return resourceHandle_;
}

// Access teardown function.
template <typename RESOURCE_HANDLE>
inline
std::function<void(RESOURCE_HANDLE) &&>
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
teardownFunc() const
{
  return teardown_;
}

// Release (teardown function will be neutralized).
template <typename RESOURCE_HANDLE>
RESOURCE_HANDLE
inline
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
release()
{
  RESOURCE_HANDLE tmp(std::move(resourceHandle_));
  resourceHandle_ = {};
  teardown_ = {};
  return tmp;
}

// Destructor.
template <typename RESOURCE_HANDLE>
inline
hep_hpc::SimpleRAII<RESOURCE_HANDLE>::
~SimpleRAII()
{
  if (teardown_) {
    teardown_(std::move(resourceHandle_));
  }
}
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Implementation of specialization.

// Constructor with arguments.
template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
inline
hep_hpc::SimpleRAII<void>::
SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args)
  :
  teardown_(teardown)
{
  setup(std::forward<ARGS>(args)...);
}

// Move constructor.
inline
hep_hpc::SimpleRAII<void>::
SimpleRAII(SimpleRAII<void> && other)
  :
  teardown_(other.teardown_)
{
  other.release();
}

// Move assignment.
inline
hep_hpc::SimpleRAII<void> &
hep_hpc::SimpleRAII<void>::
operator = (SimpleRAII<void> && other) {
  swap(*this, other);
  return *this;
}

// Access teardown function.
inline
std::function<void()>
hep_hpc::SimpleRAII<void>::
teardownFunc() const
{
  return teardown_;
}

// Release (teardown function will be neutralized).
inline
void
hep_hpc::SimpleRAII<void>::
release()
{
  teardown_ = {};
}

// Destructor.
inline
hep_hpc::SimpleRAII<void>::
~SimpleRAII()
{
  if (teardown_) {
    teardown_();
  }
}

////////////////////////////////////////////////////////////////////////
// Implementation of free swap functions.
template <typename RH>
inline
void
hep_hpc::swap(SimpleRAII<RH> & left, SimpleRAII<RH> & right)
{
  using std::swap;
  swap(left.resourceHandle_, right.resourceHandle_);
  swap(left.teardown_, right.teardown_);
}

inline
void
hep_hpc::swap(SimpleRAII<void> & left, SimpleRAII<void> & right)
{
  using std::swap;
  swap(left.teardown_, right.teardown_);
}

#endif /* hep_hpc_SimpleRAII_hpp */
