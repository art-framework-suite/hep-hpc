#ifndef hep_hpc_hdf5_ScopedErrorHandler_hpp
#define hep_hpc_hdf5_ScopedErrorHandler_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::ScopedErrorHandler
//
// Set the HDF5 error handler for the duration of the current
// scope. Previous handler will be restored on scope exit.
//
////////////////////////////////////
// NOTES:
//
// 1. If no arguments are given, HDF5 errors will not be handled.
//
// 2. Beware of possible interactions with throwing error handlers.
//
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/Utilities/SimpleRAII.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "hdf5.h"

namespace hep_hpc {
  namespace hdf5 {
    class ScopedErrorHandler;
  }
}

class hep_hpc::hdf5::ScopedErrorHandler {
public:
  // Default: no error handling for the duration.
  ScopedErrorHandler();

  // Full flexibility.
  ScopedErrorHandler(H5E_auto2_t func, void * clientData);

  // Per modes documented in hep_hpc/hdf5/errorHandling.hpp
  ScopedErrorHandler(ErrorMode mode);

private:
  hep_hpc::detail::SimpleRAII<void> errHandler_;
};

#endif /* hep_hpc_hdf5_ScopedErrorHandler_hpp */
