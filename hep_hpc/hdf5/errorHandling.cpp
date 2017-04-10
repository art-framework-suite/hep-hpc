#include "hep_hpc/hdf5/Exception.hpp"
#include "hep_hpc/Utilities/SimpleRAII.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "hdf5.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace {
  hep_hpc::hdf5::detail::StoredErrorHandler const SYSTEM_ERROR_HANDLER;
}

[[noreturn]]
herr_t
hep_hpc::hdf5::detail::throwH5Error(hid_t estack)
{
  // Resource management for error message generation.
  char * msgBuf = nullptr;
  size_t msgSize = 0ull;
  // This RAII sets up a managed buffer for subsequent cleanup, but
  // relies on it being allocated later.
  hep_hpc::detail::SimpleRAII<char *> bufMgr(msgBuf, &free);
  // This RAII allocates both its managed FILE resource and msgBuf
  // during setup, but only cleans up its own resource on
  // destruction. See above for cleanup of msgBuf, which must take
  // place *after* FILE closure.
  hep_hpc::detail::SimpleRAII<FILE *>
    fileMgr(&open_memstream, &fclose, &msgBuf, &msgSize);
  // Generate the error message using the standard HDF5 call.
  H5Eprint2(estack, *fileMgr);
  // Render our buffer usable.
  fflush(*fileMgr);
  // Throw the exception with the generated message, and clean up as
  // the stack is unwound.
  throw hep_hpc::hdf5::Exception(msgBuf);
}

herr_t
hep_hpc::hdf5::
ErrorController::setErrorHandler(ErrorMode const mode)
{
  herr_t result = -1;
  switch (mode_) {
  case ErrorMode::HDF5_DEFAULT:
    result = setErrorHandler_((H5E_auto2_t)&H5Eprint2, stderr);
    break;
  case ErrorMode::CUSTOM:
    throw std::logic_error("Mode is set to CUSTOM on call "
                           "to setErrorHandler(mode).");
  case ErrorMode::EXCEPTION:
    result = setErrorHandler_();
    break;
  case ErrorMode::NONE:
    result = setErrorHandler_();
    break;
  default:
    throw std::logic_error("INTERNAL ERROR: setErrorHandler "
                           "encountered unknown mode.");
  }
  mode_ = mode;
  return result;
}

herr_t
hep_hpc::hdf5::
ErrorController::setAndSaveErrorHandler(H5E_auto2_t const func,
                                        void * const clientData)
{
  herr_t result = -1;
  result = saveErrorHandler_();
  if (result == 0) {
    result = setErrorHandler(func, clientData);
  }
  return result;
}

herr_t
hep_hpc::hdf5::
ErrorController::setAndSaveErrorHandler(ErrorMode mode)
{
  herr_t result = saveErrorHandler_();
  if (result != 0) {
    throw Exception("Unable to save current error handling configuration.");
  }
  result = setErrorHandler(mode);
  return result;
}

herr_t
hep_hpc::hdf5::
ErrorController::resetErrorHandler()
{
  storedHandler_ = SYSTEM_ERROR_HANDLER;
  return setErrorHandler(ErrorMode::HDF5_DEFAULT);
}

herr_t
hep_hpc::hdf5::
ErrorController::restoreErrorHandler()
{
  herr_t result = -1;
  result = setErrorHandler(storedHandler_.func, storedHandler_.clientData);
  if (result == 0) {
    mode_ = storedHandler_.mode;
    storedHandler_ = SYSTEM_ERROR_HANDLER;
  }
  return result;
}

herr_t
hep_hpc::hdf5::
ErrorController::saveErrorHandler_()
{
  storedHandler_.mode = mode_;
  return H5Eget_auto2(H5E_DEFAULT, &storedHandler_.func, &storedHandler_.clientData);
}

thread_local
hep_hpc::hdf5::ErrorMode
hep_hpc::hdf5::ErrorController::mode_ {ErrorMode::HDF5_DEFAULT};

thread_local
hep_hpc::hdf5::detail::StoredErrorHandler
hep_hpc::hdf5::ErrorController::storedHandler_;
