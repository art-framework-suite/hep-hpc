#ifndef hep_hpc_Utilities_SimpleRAII_hpp
#define hep_hpc_Utilities_SimpleRAII_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::detail::SimpleRAII
//
// Simple generic RAII class template.
//
// SimpleRAII has one template argument RH, that describes
// the type of a handle to the resource to be managed.
//
// Construct:
//
// A: An optional, movable RH (non-owning, no teardown).
//
// B: 1) A movable RH.
//
//    2) A teardown function with void return type taking a (possibly
//       movable) RH as argument.
//
// C: 1) A setup function which will return an instance of a type that
//       may be stored as a RH.
//
//    2) A teardown function with void return type taking a (possibly
//       movable) RH as argument.
//
//    3) Optionally, any number of arguments to forward to the setup
//       function when invoked.
//
////////////////////////////////////
// NOTES
//
// * At construction time, the setup function (when provided) is invoked
//   with the provided arguments (if any) and the returned
//   RH is stored.
//
// * At destruction time, the teardown function is invoked with the
//   stored RH as its only argument.
//
// * A specialization is provided for the (rare) case of required paired
//   setup / teardown operations which do not provide a handle to a
//   resource (an example would be MPI initialization / finalization).
//
// * Move construction and move assignment are supported.
//
// * Dereferencing (operator *()) is supported where appropriate.
//
// * release() will cause the resource to be managed no longer (no
//   clean up).
//
// * reset() will clean up the resource. Optionally take on management
//   of a new resource with a new teardown function (c.f. constructor B,
//   above).
//
// * nothrow-correctness is problematic due to the fact that currently
//   (C++14) nothrow is not part of a function signature, and
//   std::function is not nothrow-correct.
//
////////////////////////////////////////////////////////////////////////

#include <functional>
#include <utility>

namespace hep_hpc {
  namespace detail {
    // SimpleRAII template.
    template <typename RH>
    class SimpleRAII;

    // Specialization for no resource handle.
    template <>
    class SimpleRAII<void>;

    // Free swap functions.
    template <typename RH>
    void swap(SimpleRAII<RH> & left, SimpleRAII<RH> & right);

    void swap(SimpleRAII<void> & left, SimpleRAII<void> & right);
  }
}

// Class template definition.
template <typename RH>
class hep_hpc::detail::SimpleRAII {
public:
  // Simple constructor (non-owning, no teardown).
  explicit SimpleRAII(RH rh = {});

  // Construct with resource handle and teardown function.
  template <typename TEARDOWN_FUNC>
  SimpleRAII(RH rh, TEARDOWN_FUNC teardown);

  // Construct with setup function and args, teardown function.
  template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
  SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args);

  // Free swap function.
  friend void swap<>(SimpleRAII<RH> & left, SimpleRAII<RH> & right);

  // Move constructor.
  SimpleRAII(SimpleRAII<RH> && other);

  // Move assignment.
  SimpleRAII<RH> &
  operator = (SimpleRAII<RH> && other);

  // No copy constructor.
  SimpleRAII(SimpleRAII<RH> const &) = delete;

  // No copy assignment.
  SimpleRAII<RH> &
  operator = (SimpleRAII<RH> const &) = delete;

  // Dereference operators.
  RH const & operator * () const;
  RH & operator * ();

  // Access teardown function.
  std::function<void(RH)> teardownFunc() const;

  // Release (teardown function will be neutralized).
  RH release();

  // Reset (existing resource is cleaned up).
  void reset();

  template <typename TEARDOWN_FUNC>
  void reset(RH rh, TEARDOWN_FUNC teardown);

  // Destructor.
  ~SimpleRAII();

private:
  RH resourceHandle_ {};
  std::function<void(RH)> teardown_ {};
};

// Class template specialization definition.
template <>
class hep_hpc::detail::SimpleRAII<void> {
public:
  // Default constructor.
  SimpleRAII() noexcept = default;

  // Construct with teardown function.
  template <typename TEARDOWN_FUNC>
  explicit 
  SimpleRAII(TEARDOWN_FUNC teardown);

  // Construct with setup function and args, teardown function.
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

  // Reset (teardown will be called).
  void reset();

  template <typename TEARDOWN_FUNC>
  void reset(TEARDOWN_FUNC teardown);

  // Destructor.
  ~SimpleRAII();

private:
  std::function<void()> teardown_ {};
};

////////////////////////////////////////////////////////////////////////
// Implementation of class template.

// Simple constructor (non-owning, no teardown).
template <typename RH>
inline
hep_hpc::detail::SimpleRAII<RH>::
SimpleRAII(RH rh)
  :
  resourceHandle_{std::move(rh)},
  teardown_{}
{
}

// Construct with resource handle and teardown function.
template <typename RH>
template <typename TEARDOWN_FUNC>
inline
hep_hpc::detail::SimpleRAII<RH>::
SimpleRAII(RH rh, TEARDOWN_FUNC teardown)
  :
  resourceHandle_(std::move(rh)),
  teardown_(teardown)
{
}

// Construct with setup function and args, teardown function.
template <typename RH>
template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
inline
hep_hpc::detail::SimpleRAII<RH>::
SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args)
  :
  resourceHandle_(setup(std::forward<ARGS>(args)...)),
  teardown_(teardown)
{
}

// Move constructor.
template <typename RH>
inline
hep_hpc::detail::SimpleRAII<RH>::
SimpleRAII(SimpleRAII<RH> && other)
  :
  resourceHandle_(std::move(other.resourceHandle_)),
  teardown_(std::move(other.teardown_))
{
  (void) other.release();
}

// Move assignment.
template <typename RH>
inline
hep_hpc::detail::SimpleRAII<RH> &
hep_hpc::detail::SimpleRAII<RH>::
operator = (SimpleRAII<RH> && other) {
  swap(*this, other);
  return *this;
}

// Dereference operators.
template <typename RH>
inline
RH const &
hep_hpc::detail::SimpleRAII<RH>::
operator * () const
{
  return resourceHandle_;
}

template <typename RH>
inline
RH &
hep_hpc::detail::SimpleRAII<RH>::
operator * ()
{
  return resourceHandle_;
}

// Access teardown function.
template <typename RH>
inline
std::function<void(RH)>
hep_hpc::detail::SimpleRAII<RH>::
teardownFunc() const
{
  return teardown_;
}

// Release (teardown function will be neutralized).
template <typename RH>
RH
inline
hep_hpc::detail::SimpleRAII<RH>::
release()
{
  RH tmp(std::move(resourceHandle_));
  // Avoid cleanup.
  resourceHandle_ = {};
  teardown_ = {};
  return tmp;
}

// Reset (resource will be cleaned up).
template <typename RH>
inline
void
hep_hpc::detail::SimpleRAII<RH>::
reset()
{
  using std::swap;
  SimpleRAII<RH> tmp;
  swap(*this, tmp);
}

// Reset (resource will be cleaned up).
template <typename RH>
template <typename TEARDOWN_FUNC>
inline
void
hep_hpc::detail::SimpleRAII<RH>::
reset(RH rh, TEARDOWN_FUNC teardown)
{
  using std::swap;
  SimpleRAII<RH> tmp(rh, teardown);
  swap(*this, tmp);
}

// Destructor.
template <typename RH>
inline
hep_hpc::detail::SimpleRAII<RH>::
~SimpleRAII()
{
  if (teardown_) {
    teardown_(std::move(resourceHandle_));
  }
}
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Implementation of specialization.

// Construct with teardown function.
template <typename TEARDOWN_FUNC>
inline
hep_hpc::detail::SimpleRAII<void>::
SimpleRAII(TEARDOWN_FUNC teardown)
  :
  teardown_(teardown)
{
}

// Construct with setup function and args, teardown function.
template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename ... ARGS>
inline
hep_hpc::detail::SimpleRAII<void>::
SimpleRAII(SETUP_FUNC setup, TEARDOWN_FUNC teardown, ARGS && ... args)
  :
  teardown_(teardown)
{
  setup(std::forward<ARGS>(args)...);
}

// Move constructor.
inline
hep_hpc::detail::SimpleRAII<void>::
SimpleRAII(SimpleRAII<void> && other)
  :
  teardown_(other.teardown_)
{
  other.release();
}

// Move assignment.
inline
hep_hpc::detail::SimpleRAII<void> &
hep_hpc::detail::SimpleRAII<void>::
operator = (SimpleRAII<void> && other) {
  swap(*this, other);
  return *this;
}

// Access teardown function.
inline
std::function<void()>
hep_hpc::detail::SimpleRAII<void>::
teardownFunc() const
{
  return teardown_;
}

// Release (teardown function will be neutralized).
inline
void
hep_hpc::detail::SimpleRAII<void>::
release()
{
  teardown_ = {};
}

// Reset (teardown() will be called).
inline
void
hep_hpc::detail::SimpleRAII<void>::
reset()
{
  using std::swap;
  SimpleRAII<void> tmp;
  swap(*this, tmp);
}

// Reset (existing teardown will be called).
template <typename TEARDOWN_FUNC>
inline
void
hep_hpc::detail::SimpleRAII<void>::
reset(TEARDOWN_FUNC teardown)
{
  using std::swap;
  SimpleRAII<void> tmp(teardown);
  swap(*this, tmp);
}

// Destructor.
inline
hep_hpc::detail::SimpleRAII<void>::
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
hep_hpc::detail::swap(SimpleRAII<RH> & left, SimpleRAII<RH> & right)
{
  using std::swap;
  swap(left.resourceHandle_, right.resourceHandle_);
  swap(left.teardown_, right.teardown_);
}

inline
void
hep_hpc::detail::swap(SimpleRAII<void> & left, SimpleRAII<void> & right)
{
  using std::swap;
  swap(left.teardown_, right.teardown_);
}

#endif /* hep_hpc_Utilities_SimpleRAII_hpp */
