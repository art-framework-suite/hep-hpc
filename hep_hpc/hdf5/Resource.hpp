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
////////////////////////////////////
// NOTES
//
// * Valid only for hid_t and HID_t types.
//
/////////////////////////////////////////////////////////////////////////

#include "hep_hpc/Utilities/SimpleRAII.hpp"
#include "hep_hpc/hdf5/HID_t.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "hdf5.h"

#include <type_traits>

namespace hep_hpc {
  namespace hdf5 {
    template <typename RH,
              typename = std::enable_if_t<std::is_same<RH, HID_t>::value ||
                                          std::is_same<RH, hid_t>::value> >
    class Resource;

    template <typename RH>
    void swap(Resource<RH> & left, Resource<RH> & right);

  }
}

template <typename RH, typename>
  class hep_hpc::hdf5::Resource : hep_hpc::detail::SimpleRAII<RH> {
public:
  using base = hep_hpc::detail::SimpleRAII<RH>;

  // Construct with resource handle (non-owning).
  explicit Resource(RH rh = {})
    : base(rh)
    {
    }

  // Construct with resource handle and teardown function.
  template <typename TEARDOWN_FUNC>
  Resource(RH rh, TEARDOWN_FUNC teardown)
    : base(rh,
           [teardown](RH rh) { return ErrorController::call(teardown, rh); })
    {
    }

  // Construct with setup function and args, teardown function.
  template <typename SETUP_FUNC, typename TEARDOWN_FUNC, typename... Args>
  Resource(SETUP_FUNC setup, TEARDOWN_FUNC teardown, Args && ... args)
    : base([&]() { return ErrorController::call(setup, std::forward<Args>(args)...); },
           [teardown](RH rh) { return ErrorController::call(teardown, rh); })
    {
    }

  // Move constructor.
  Resource(Resource<RH> && other)
    : base(std::move(other))
    {
    }

  // Move assignment.
  Resource<RH> &
  operator = (Resource<RH> && other)
    {
      using std::swap;
      swap(*this, other);
      return *this;
    }

  using base::operator *;
  using base::teardownFunc;
  using base::release;
  using base::reset;

  friend void swap<>(Resource<RH> & left, Resource<RH> & right);

};

template <typename RH>
void
hep_hpc::hdf5::swap(Resource<RH> & left, Resource<RH> & right)
{
  swap(dynamic_cast<typename Resource<RH>::base &>(left),
       dynamic_cast<typename Resource<RH>::base &>(right));
}


#endif /* hep_hpc_hdf5_Resource_hpp */
