#include "hep_hpc/H5Exception.hpp"
#include "hep_hpc/SimpleRAII.hpp"
#include "hep_hpc/errorHandling.hpp"

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
    hep_hpc::SimpleRAII<char **>
      bufMgr([](char * & msgBuf){ return &msgBuf; },
             [](auto && bufPtr){ if (bufPtr != nullptr) { free(*bufPtr); } },
             msgBuf);
    // This RAII allocates both its managed FILE resource and msgBuf
    // during setup, but only cleans up its own resource on
    // destruction. See above for cleanup of msgBuf, which must take
    // place *after* FILE closure.
    hep_hpc::SimpleRAII<FILE *>
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
    throw hep_hpc::H5Exception(msgBuf);
  }

  struct StoredErrorHandler {
    StoredErrorHandler() : func((H5E_auto2_t)(&H5Eprint2)), clientData(stderr) { }
    H5E_auto2_t func;
    void *clientData;
  };

  StoredErrorHandler const SYSTEM_ERROR_HANDLER;
}

void
hep_hpc::initH5ErrorHandling()
{
  H5Eset_auto2(H5E_DEFAULT, &throwH5Error, NULL);
}

void hep_hpc::resetErrorHandler()
{
  H5Eset_auto2(H5E_DEFAULT,
               SYSTEM_ERROR_HANDLER.func,
               SYSTEM_ERROR_HANDLER.clientData);
}

namespace {
  thread_local StoredErrorHandler savedErrorHandler;
}

void hep_hpc::setAndSaveErrorHandler(H5E_auto2_t func, void * clientData)
{
  H5Eget_auto2(H5E_DEFAULT, &savedErrorHandler.func, &savedErrorHandler.clientData);
  H5Eset_auto2(H5E_DEFAULT, func, clientData);
}

void hep_hpc::restoreErrorHandler()
{
  H5Eset_auto2(H5E_DEFAULT, savedErrorHandler.func, savedErrorHandler.clientData);
  savedErrorHandler = SYSTEM_ERROR_HANDLER;
}
