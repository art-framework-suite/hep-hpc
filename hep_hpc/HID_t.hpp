#ifndef hep_hpc_HID_t_hpp
#define hep_hpc_HID_t_hpp
////////////////////////////////////////////////////////////////////////
// HID_t
//
// Wrapped hid_t which has the value -1 when default-constructed. Useful
// in resource management (see SimpleRAII.hpp).
//
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/detail/DefaultedSimpleType.hpp"

#include "hdf5.h"

namespace hep_hpc {
  using HID_t = detail::DefaultedSimpleType<hid_t, -1ll>;
}

#endif /* hep_hpc_HID_t_hpp */
