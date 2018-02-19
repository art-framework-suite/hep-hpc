#ifndef hep_hpc_hdf5_Resource_hpp
#define hep_hpc_hdf5_Resource_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::Resource.
//
// Class template to facilitate management of HDF5 resources.
//
// This class will execute the specified setup function with the
// provided arguments in the context of
// hep_hpc::hdf5::ErrorController::call(...), storing the resulting
// resource handle. Upon destruction, the specified teardown function is
// called (again in the context of
// hep_hpc::hdf5::ErrorController::call(...)) to clean up the
// previously-allocated resource.
//
/////////////////////////////////////////////////////////////////////////

#include "hep_hpc/Utilities/SimpleRAII.hpp"
#include "hep_hpc/hdf5/HID_t.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "hdf5.h"

#include <type_traits>

namespace hep_hpc {
  namespace hdf5 {
    class Resource;

    void swap(Resource & left, Resource & right);

  }
}

class hep_hpc::hdf5::Resource : hep_hpc::detail::SimpleRAII<HID_t> {
public:
  using base = hep_hpc::detail::SimpleRAII<HID_t>;

  // Construct with resource handle (non-owning).
  explicit Resource(HID_t rh = {})
    : base(rh)
    {
    }

  // Construct with resource handle and teardown function.
  template <typename TEARDOWN_FUNC>
  Resource(HID_t rh, TEARDOWN_FUNC teardown)
    : base(rh,
           [teardown](HID_t rh) { return ErrorController::call(teardown, rh); })
    {
    }

  template <typename TEARDOWN_FUNC>
  Resource(hid_t rh, TEARDOWN_FUNC teardown)
    : base(HID_t(rh),
           [teardown](HID_t rh) { return ErrorController::call(teardown, rh); })
    {
    }

  // Construct with setup function and args, teardown function.
  template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename... Args>
  Resource(SETUP_FUNC setup, TEARDOWN_FUNC teardown, Args && ... args)
    : base([&]() { return ErrorController::call(setup, std::forward<Args>(args)...); },
           [teardown](HID_t rh) { return ErrorController::call(teardown, rh); })
    {
    }

  // Move constructor.
  Resource(Resource && other)
    : base(std::move(other))
    {
    }

  // Move assignment.
  Resource &
  operator = (Resource && other)
    {
      using std::swap;
      swap(*this, other);
      return *this;
    }

  using base::operator *;
  using base::teardownFunc;
  using base::release;
  using base::reset;

  friend void swap(Resource & left, Resource & right);

};

inline
void
hep_hpc::hdf5::swap(Resource & left, Resource & right)
{
  swap(static_cast<typename Resource::base &>(left),
       static_cast<typename Resource::base &>(right));
}


#endif /* hep_hpc_hdf5_Resource_hpp */
