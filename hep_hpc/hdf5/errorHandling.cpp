#include "hep_hpc/hdf5/Exception.hpp"
#include "hep_hpc/Utilities/SimpleRAII.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "hdf5.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace {
  [[noreturn]]
  herr_t throwH5Error(hid_t estack, void *)
  {
    // Deactivate error handling for this stack. If the exception is
    // caught, error handling should be reenabled using a further call
    // to initH5ErrorHandling().
    H5Eset_auto(estack, NULL, NULL);
    // Resource management for error message generation.
    char * msgBuf = nullptr;
    size_t msgSize = 0ull;
    // This RAII sets up a managed buffer for subsequent cleanup, but
    // relies on it being allocated later.
    hep_hpc::detail::SimpleRAII<char **>
      bufMgr([](char * & msgBuf){ return &msgBuf; },
             [](auto && bufPtr){ if (bufPtr != nullptr) { free(*bufPtr); } },
             msgBuf);
    // This RAII allocates both its managed FILE resource and msgBuf
    // during setup, but only cleans up its own resource on
    // destruction. See above for cleanup of msgBuf, which must take
    // place *after* FILE closure.
    hep_hpc::detail::SimpleRAII<FILE *>
      fileMgr([](char * & msgBuf, size_t & msgSize)
              {
                return open_memstream(&msgBuf, &msgSize);
              },
              [](FILE * && file){ fclose(file); },
              msgBuf,
              msgSize);
    // Generate the error message using the standard HDF5 call.
    H5Eprint2(estack, *fileMgr);
    // Render our buffer usable.
    fflush(*fileMgr);
    // Throw the exception with the generated message, and clean up as
    // the stack is unwound.
    throw hep_hpc::hdf5::Exception(msgBuf);
  }

  struct StoredErrorHandler {
    StoredErrorHandler() : func((H5E_auto2_t)(&H5Eprint2)), clientData(stderr) { }
    H5E_auto2_t func;
    void *clientData;
  };

  StoredErrorHandler const SYSTEM_ERROR_HANDLER;
}

herr_t
hep_hpc::hdf5::setErrorHandler(ErrorMode mode)
{
  herr_t result = -1;
  switch (mode) {
  case ErrorMode::HDF5_DEFAULT:
    result = setErrorHandler((H5E_auto2_t)&H5Eprint2, stderr);
    break;
  case ErrorMode::EXCEPTIONS:
    result = setErrorHandler(&throwH5Error, nullptr);
    break;
  case ErrorMode::NONE:
    result = setErrorHandler();
    break;
  default:
    throw std::runtime_error("INTERNAL ERROR: setErrorHandler encountered unknown mode.");
  }
  return result;
}

namespace {
  thread_local StoredErrorHandler savedErrorHandler;
}

herr_t
hep_hpc::hdf5::setAndSaveErrorHandler(H5E_auto2_t func, void * clientData)
{
  herr_t result = -1;
  result = H5Eget_auto2(H5E_DEFAULT, &savedErrorHandler.func, &savedErrorHandler.clientData);
  if (result == 0) {
    result = setErrorHandler(func, clientData);
  }
  return result;
}

herr_t hep_hpc::hdf5::setAndSaveErrorHandler(ErrorMode mode)
{
  herr_t result = -1;
  result = H5Eget_auto2(H5E_DEFAULT, &savedErrorHandler.func, &savedErrorHandler.clientData);
  if (result == 0) {
    result = setErrorHandler(mode);
  }
  return result;
}

herr_t hep_hpc::hdf5::resetErrorHandler()
{
 return setErrorHandler(ErrorMode::HDF5_DEFAULT);
}

herr_t hep_hpc::hdf5::restoreErrorHandler()
{
  herr_t result = -1;
  result = setErrorHandler(savedErrorHandler.func, savedErrorHandler.clientData);
  if (result == 0) {
    savedErrorHandler = SYSTEM_ERROR_HANDLER;
  }
  return result;
}
