#ifndef hep_hpc_ScopedErrorHandler_hpp
#define hep_hpc_ScopedErrorHandler_hpp
////////////////////////////////////////////////////////////////////////
// ScopedErrorHandler
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

#include "hep_hpc/SimpleRAII.hpp"
#include "hep_hpc/errorHandling.hpp"
////////////////////////////////////////////////////////////////////////
// hep_hpc::ScopedErrorHandler.
//
// Set the specified mode of error handling for HDF5 errors for the
// current scope, restoring the previous error handling scheme on exit
// from scope.
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/errorHandling.hpp"

#include "hdf5.h"

namespace hep_hpc {
  class ScopedErrorHandler;
}

class hep_hpc::ScopedErrorHandler {
public:
  // Default: no error handling for the duration.
  ScopedErrorHandler();

  // Full flexibility.
  ScopedErrorHandler(H5E_auto2_t func, void * clientData);

  // Per modes documented in hep_hpc/errorHandling.hpp
  ScopedErrorHandler(ErrorMode mode);

private:
  SimpleRAII<void> errHandler_;
};
#endif /* hep_hpc_ScopedErrorHandler_hpp */
