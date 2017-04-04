#ifndef hep_hpc_hdf5_HID_t_hpp
#define hep_hpc_hdf5_HID_t_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::HID_t
//
// Wrapped hid_t which has the value -1 when default-constructed. Useful
// in resource management (see hep_hpc/Utilities/SimpleRAII.hpp).
//
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/Utilities/DefaultedSimpleType.hpp"

#include "hdf5.h"

namespace hep_hpc {
  namespace hdf5 {
    using HID_t = detail::DefaultedSimpleType<hid_t, -1ll>;
  }
}

#endif /* hep_hpc_hdf5_HID_t_hpp */
