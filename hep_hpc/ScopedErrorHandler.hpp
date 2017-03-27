#ifndef NNNHYEDTHFUI
#define NNNHYEDTHFUI
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

#include "hdf5.h"

namespace hep_hpc {
  class ScopedErrorHandler;
}

class hep_hpc::ScopedErrorHandler {
public:
  ScopedErrorHandler();
  ScopedErrorHandler(H5E_auto2_t func, void * clientData);

private:
  SimpleRAII<void> errHandler_;
};
#endif
