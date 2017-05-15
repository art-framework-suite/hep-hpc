#ifndef hep_hpc_hdf5_errorHandling_hpp
#define hep_hpc_hdf5_errorHandling_hpp
////////////////////////////////////////////////////////////////////////
// HDF5 Error handling facilities.
//
// Various entities for controlling the behavior of interactions with
// the HDF5 system.
//
////////////////////////////////////
// enum class hep_hpc::hdf5::ErrorMode;
//
// An enumerator class describing how HDF5 errors should be
// handled. Valid values are:
//
//   * HDF5_DEFAULT: this is the standard HDF5 behavior: print an error
//     message and return a non-SUCCESS code.
//
//   * CUSTOM: this indicates that a user-provided HDF5 error handler is
//     configured. Note that this value is used internally, and should
//     not be provided as a parameter by the user to any public
//     functions.
//
//   * EXCEPTION: this will cause (suitably wrapped) HDF5 functions in
//     the current thread to throw an exception (see
//     hdf5study::hdf5::ErrorController::call(...) for details and
//     general notes, below.
//
//   * NONE: Error handling must be done on a case-by-case basis by the
//     user based on return codes.
//
////////////////////////////////////
// class hdf5study::hdf5::ErrorController;
//
// A singleton class with only static members providing various
// functions for controlling the behavior of the HDF5 library in a C++
// context.
//
// Public member functions.
//
// herr_t setErrorHandler(ErrorMode mode);
// herr_t setAndSaveErrorHandler(ErrorMode mode);
//
//   Configure the HDF5 error handling to one of the preset modes
//   described above. The second signature also saves the current state
//   of the error handling configuration for later restoration with
//   restoreErrorHandler().
//
// herr_t setErrorHandler(H5E_auto2_t errorFunc, void * clientData);
// herr_t setAndSaveErrorHandler(H5E_auto2_t errorFunc, void * clientData);
//
//   Set the custom HDF5 error handling function to be called prior to
//   return from public-facing HDF5 functions. errorFunc must be of type
//   H5E_auto2_t, which is:
//
//     herr_t (*) (hid_t estack, void *client_data);
//
//   The second signature also saves the current state of the error
//   handling configuration for later restoration with
//   restoreErrorHandler().
//
// herr_t resetErrorHandler();
//
//   Reset the HDF5 error handling to its default, which calls the HDF5
//   routine H5Eprint to print the current HDF5 error stack to
//   stderr. Any stored error handling configuration is also reset.
//
// herr_t restoreErrorHandler();
//
//   Restore the saved error handling configuration. If no error
//   handling configuration is saved, this is identical to calling
//   resetErrorHandler().
//
// <ret> call(H5FUNC h5func, Args && ... args);
// <ret> call(ErrorMode mode, H5FUNC h5func, Args && ... args);
// <ret> call(H5E_auto2_t errorFunc, void * clientData,
//            H5FUNC h5func, Args && ... args);
//
//   Call the provided function with the provided arguments. If the
//   returned value is < 0 and the error handling mode is
//   ErrorMode::EXCEPTION, then an exception is thrown containing the
//   HDF5 error stack information.
//
//   The second and third signatures are equivalent to calling
//   setAndSaveErrorHandler(...) prior to calling and
//   restoreErrorHandler() afterwards (even if an exception is thrown --
//   see ScopedErrorHandler, below.).
//
////////////////////////////////////
// class ScopedErrorHandler;
//
// A class to facilitate the temporary setting of the error handling
// configuration within a particular scope. Previous error handling
// configuration will be restored at scope exit.
//
// Constructors.
//
// ScopedErrorHandler(ErrorMode mode = ErrorMode::NONE);
// ScopedErrorHandler(H5E_auto2_t func, void * clientData);
//
//   Set the error handling configuration as specified.
//
////////////////////////////////////
// NOTES
// 
// * The implementation of the exception-throwing functionality requires
//   POSIX.1-2008 support, specifically the function
//   open_memstream(). If missing from the system, it is supplied in the
//   memstream/ directory of this package.
//
// * The ability to throw an exception upon HDF5 failure does not
//   utilize the HDF5 error-handling callback system (other than to
//   disable it), and is therefore safe from the point of view of HDF5
//   library integrity since it does not interrupt intra-HDF5-library
//   control flow. In order to operate as desired however, it does
//   require that HDF5 calls be wrapped, either via
//   hep_hpc::hdf5::Resource<...> (see hep_hpc/hdf5/Resource.hpp) or
//   via hep_hpc::hdf5::ErrorController::call(...).
//
// * For consistent HDF5 error handling behavior, all user-space calls
//   to HDF5 functions which may conceivably invoke the HDF5 error
//   handling system should be done via
//   hep_hpc::hdf5::ErrorController::call(...).
//
// * Interactions with hep_hpc::hdf5::ErrorController are thread-safe,
//   inasmuch as you are using an HDF5 library configured for thread
//   safety, as error handling is configured on a per-thread basis. User
//   is responsible for ensuring that any exceptions thrown by the error
//   handling system described here do not propagate outside the current
//   thread.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/Utilities/SimpleRAII.hpp"
#include "hep_hpc/hdf5/Exception.hpp"

#include "hdf5.h"

#include <cstdint>

namespace hep_hpc {
  namespace hdf5 {

    class ErrorController;

    class ScopedErrorHandler;

    namespace detail {
      enum class ErrorMode : uint8_t {
        HDF5_DEFAULT, // H5Eprint.
          CUSTOM, // User-provided error handler.
          EXCEPTION, // C++ exception containing output of H5Eprint.
          NONE // No error-handling.
          };
    } // namespace detail.

    using detail::ErrorMode; // All reference to enumerator values must
                             // be qualified.

    namespace detail {
      struct StoredErrorHandler {
        StoredErrorHandler() = default;

        StoredErrorHandler(ErrorMode mode,
                           H5E_auto2_t func,
                           void * clientData) noexcept
          :
          mode(mode),
          func(func),
          clientData(clientData)
          { }

        ErrorMode mode {ErrorMode::HDF5_DEFAULT};
        H5E_auto2_t func {(H5E_auto2_t)&H5Eprint2};
        void *clientData {stderr};
      };

      bool operator == (StoredErrorHandler const & left,
                        StoredErrorHandler const & right);

      StoredErrorHandler saveErrorHandler();

      [[noreturn]] herr_t throwH5Error(std::string msg = "",
                                       hid_t estack = H5E_DEFAULT);
    } // namespace detail.

  } // namespace hdf5.
} // namespace hep_hpc.

class hep_hpc::hdf5::ErrorController {
public:

  static herr_t setErrorHandler(H5E_auto2_t errorFunc = nullptr,
                                void * clientData = nullptr);

  static herr_t setErrorHandler(ErrorMode mode);

  static herr_t setAndSaveErrorHandler(H5E_auto2_t errorFunc = nullptr,
                                       void * clientData = nullptr);

  static herr_t setAndSaveErrorHandler(ErrorMode mode);
  
  static herr_t resetErrorHandler();

  static void saveErrorHandler();

  static herr_t restoreErrorHandler();

  static ErrorMode currentMode();

  // No leading message.
  template <typename H5FUNC, typename... Args>
  static auto call(H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...));

  template <typename H5FUNC, typename... Args>
  static auto call(ErrorMode mode,
                             H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...));

  template <typename H5FUNC, typename... Args>
  static auto call(H5E_auto2_t errorFunc, void * clientData,
                             H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...));

  // Exception will start with specified message.
  template <typename H5FUNC, typename... Args>
  static auto call(std::string msg,
                             H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...));

  template <typename H5FUNC, typename... Args>
  static auto call(ErrorMode mode,
                             std::string msg,
                             H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...));

  template <typename H5FUNC, typename... Args>
  static auto call(H5E_auto2_t errorFunc, void * clientData,
                             std::string msg,
                             H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...));


private:
  ErrorController() = default;
  ~ErrorController() = default;

  static herr_t setErrorHandler_(H5E_auto2_t errorFunc = nullptr,
                                 void * clientData = nullptr);

  static thread_local ErrorMode mode_;
  static thread_local detail::StoredErrorHandler storedHandler_;
};

class hep_hpc::hdf5::ScopedErrorHandler {
public:
  // Per modes.
  ScopedErrorHandler(ErrorMode mode = ErrorMode::NONE);

  // Full flexibility.
  ScopedErrorHandler(H5E_auto2_t func, void * clientData);

private:
  hep_hpc::detail::SimpleRAII<void> errHandler_;
};

inline
herr_t
hep_hpc::hdf5::ErrorController::
setErrorHandler(H5E_auto2_t func, void * clientData)
{
  mode_ = (func == nullptr) ? ErrorMode::NONE : ErrorMode::CUSTOM;
  return setErrorHandler_(func, clientData);
}

inline
herr_t
hep_hpc::hdf5::ErrorController::
setAndSaveErrorHandler(H5E_auto2_t const func, void * const clientData)
{
  saveErrorHandler();
  return setErrorHandler(func, clientData);
}

inline
herr_t
hep_hpc::hdf5::ErrorController::
setAndSaveErrorHandler(ErrorMode mode)
{
  saveErrorHandler();
  return setErrorHandler(mode);
}

inline
void
hep_hpc::hdf5::ErrorController::
saveErrorHandler()
{
  storedHandler_ = detail::saveErrorHandler();
}

inline
hep_hpc::hdf5::ErrorMode
hep_hpc::hdf5::ErrorController::
currentMode()
{
  return mode_;
}

template <typename H5FUNC, typename... Args>
auto
hep_hpc::hdf5::ErrorController::
call(H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...))
{
  using namespace std::string_literals;
  return call(""s, h5func, std::forward<Args>(args)...);
}

template <typename H5FUNC, typename... Args>
inline
auto
hep_hpc::hdf5::ErrorController::
call(ErrorMode mode,
     H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...))
{
  using namespace std::string_literals;
  return call(mode, ""s, h5func, std::forward<Args>(args)...);
}

template <typename H5FUNC, typename... Args>
inline
auto
hep_hpc::hdf5::ErrorController::
call(H5E_auto2_t errorFunc, void * clientData,
     H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...))
{
  using namespace std::string_literals;
  return call(errorFunc, clientData, ""s, h5func, std::forward<Args>(args)...);
}

template <typename H5FUNC, typename... Args>
auto
hep_hpc::hdf5::ErrorController::
call(std::string msg,
     H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...))
{
  decltype(h5func(std::forward<Args>(args)...)) result =
    h5func(std::forward<Args>(args)...);
  if (result < 0 &&
      mode_ == ErrorMode::EXCEPTION &&
      !std::uncaught_exception()) {
    detail::throwH5Error(std::move(msg));
  }
  return result;
}

template <typename H5FUNC, typename... Args>
inline
auto
hep_hpc::hdf5::ErrorController::
call(ErrorMode mode,
     std::string msg,
     H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...))
{
  decltype(h5func(std::forward<Args>(args)...)) result;
  // FIXME: Need std::optional, here (C++17).
  if (mode_ == mode) {
    result = call(std::move(msg), h5func, std::forward<Args>(args)...);
  } else {
    ScopedErrorHandler seh(mode);
    result = call(std::move(msg), h5func, std::forward<Args>(args)...);
  }
  return result;
}

template <typename H5FUNC, typename... Args>
inline
auto
hep_hpc::hdf5::ErrorController::
call(H5E_auto2_t errorFunc, void * clientData,
     std::string msg,
     H5FUNC h5func, Args && ... args)
-> decltype(h5func(std::forward<Args>(args)...))
{
  decltype(h5func(std::forward<Args>(args)...)) result;
  ScopedErrorHandler seh(errorFunc, clientData);
  result = call(std::move(msg), h5func, std::forward<Args>(args)...);
  return result;
}

inline
herr_t
hep_hpc::hdf5::ErrorController::
setErrorHandler_(H5E_auto2_t func, void * clientData)
{
  return call(&H5Eset_auto2, H5E_DEFAULT, func, clientData);
}

inline
hep_hpc::hdf5::ScopedErrorHandler::
ScopedErrorHandler(ErrorMode mode)
  :
  errHandler_(static_cast<herr_t(*)(ErrorMode)>(&ErrorController::setAndSaveErrorHandler),
              &ErrorController::restoreErrorHandler, mode)
{
}

inline
hep_hpc::hdf5::ScopedErrorHandler::
ScopedErrorHandler(H5E_auto2_t func, void * clientData)
  :
  errHandler_(static_cast<herr_t(*)(H5E_auto2_t, void *)>(&ErrorController::setAndSaveErrorHandler),
              &ErrorController::restoreErrorHandler, func, clientData)
{
}


inline
bool
hep_hpc::hdf5::detail::
operator == (StoredErrorHandler const & left,
             StoredErrorHandler const & right)
{
  return left.mode == right.mode &&
    left.func == right.func &&
    left.clientData == right.clientData;
}

#endif /* hep_hpc_hdf5_errorHandling_hpp */
