#ifndef hep_hpc_errorHandling_hpp
#define hep_hpc_errorHandling_hpp
////////////////////////////////////////////////////////////////////////
// HDF5 Error handling functions.
//
////////////////////////////////////
// setErrorHandler(H5E_auto2_t func = nullptr, void* clientData = nullptr)
// setErrorHandler(ErrorMode::(HDF5_DEFAULT|EXCEPTIONS|NONE))
//
// Unconditionally initialize the HDF5 error handling system to the
// specified handler or mode. Valid modes:
//
//   * HDF5_DEFAULT: this is the standard HDF5 behavior: print an error
//     message and return a non-SUCCESS code.
//
//   * EXCEPTIONS: this will cause HDF5 functions in the current thread
//     to throw an exception.
//
//   * NONE: Error handling must be done on a case-by-case basis by the
//     user based on return codes.
//
// N.B. The implementation of the exception-throwing functionality
// requires POSIX.1-2008 support, specifically the function
// open_memstream(). If support for other platforms is required,
// consider using a third party implementation such as
// http://piumarta.com/software/memstream/,
//
// N.M.B. It is unclear whether this functionality conflicts with the
// note regarding callbacks in the documentation of the H5Eset_auto()
// HDF5 function. See
// https://support.hdfgroup.org/HDF5/doc1.8/RM/RM_H5E.html#Error-SetAuto
// for more details.
//
////////////////////////////////////
// setAndSaveErrorHandler(H5E_auto2_t func = nullptr, void* clientData = nullptr)
// setAndSaveErrorHandler(ErrorMode::(HDF5_DEFAULT|EXCEPTIONS|NONE))
//
// Set the HDF5 error handler for the current error stack, saving the
// existing settings.
//
////////////////////////////////////
// resetErrorHandler().
//
// Unconditionally reset the HDF5 error handler to the system defaults
// (equivalent to setErrorHandler(ErrorMode::HDF5_DEFAULT)).
//
////////////////////////////////////
// restoreErrorHandler()
//
// Restore this thread's previously-saved HDF5 error handler. If nothing
// was saved, this is equivalent to resetErrorHandler().
//
////////////////////////////////////////////////////////////////////////

#include "hdf5.h"

#include <cstdint>

namespace hep_hpc {
  namespace detail {
    enum class ErrorMode : uint8_t {
      HDF5_DEFAULT, // H5Eprint
        EXCEPTIONS, // C++ exception containing output of H5Eprint.
        NONE // No error-handling.
        };
  }

  using detail::ErrorMode;

  herr_t setErrorHandler(H5E_auto2_t func = nullptr, void * clientData = nullptr);
  herr_t setErrorHandler(ErrorMode mode);

  herr_t setAndSaveErrorHandler(H5E_auto2_t func = nullptr, void * clientData = nullptr);
  herr_t setAndSaveErrorHandler(ErrorMode mode);

  herr_t resetErrorHandler();

  herr_t restoreErrorHandler();
}

inline
herr_t
hep_hpc::setErrorHandler(H5E_auto2_t func, void * clientData)
{
  return H5Eset_auto2(H5E_DEFAULT, func, clientData);
}

#endif /* hep_hpc_errorHandling_hpp */
