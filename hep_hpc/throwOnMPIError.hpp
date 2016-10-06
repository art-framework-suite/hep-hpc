#ifndef HDFSTUDY_THROWONMPIERROR_HPP
#define HDFSTUDY_THROWONMPIERROR_HPP
////////////////////////////////////////////////////////////////////////
// hep_hpc::throwOnMPIError()
//
// Throw an exception upon non-successful return from an MPI function.
//
// Signatures:
//
// 1. void throwOnMPIError(<name-of-func>,
//                         <pointer-to-func>[, <args>...]);
//
// 2. void throwOnMPIError(<pointer-to-func>[, <args>...]);
//
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/detail/demangle_symbol.hpp"
#include "hep_hpc/detail/throwOnMPIError.hpp"

#include <string>
#include <type_traits>
#include <typeinfo>

namespace hep_hpc {
  template <typename MPI_FUNC, typename ... ARGS>
  typename
  std::enable_if<std::is_function<typename
                                  std::remove_pointer<MPI_FUNC>::type>::value,
                 void>::type
  throwOnMPIError(MPI_FUNC func, ARGS && ... args);

  template <typename MPI_FUNC, typename ... ARGS>
  void throwOnMPIError(std::string const & funcString,
                       MPI_FUNC func,
                       ARGS && ... args);
}

#ifndef NDEBUG
// Check the assumption that 0 == MPI_SUCCESS that we make in order to
// avoid requiring mpi.h in this header for the short-circuit test in
// throwOnMPI which avoids a function call in the majority of cases.
#include "mpi.h"
static_assert(0 == MPI_SUCCESS, "MPI_SUCCESS MUST BE 0!");
#endif

template <typename MPI_FUNC, typename ... ARGS>
typename
std::enable_if<std::is_function<typename
                                std::remove_pointer<MPI_FUNC>::type>::value,
               void>::type
hep_hpc::
throwOnMPIError(MPI_FUNC func, ARGS && ... args)
{
  int status = func(std::forward<ARGS>(args)...);
  if (status != 0) { // See note above regarding MPI_SUCCESS.
    detail::throwOnMPIError(detail::demangle_symbol(typeid(func).name()), status);
  }
}

// Code reuse between these two functions is sacrificed in order to
// avoid an unconditional call to demangle_symbol which would be
// unnecessary if the function did not return an error.
template <typename MPI_FUNC, typename ... ARGS>
inline
void
hep_hpc::throwOnMPIError(std::string const & funcString,
                          MPI_FUNC func,
                          ARGS && ... args)
{
  int status = func(std::forward<ARGS>(args)...);
  if (status != 0) { // See note above regarding MPI_SUCCESS.
    detail::throwOnMPIError(funcString, status);
  }
}
#endif /* HDFSTUDY_THROWONMPIERROR_HPP */
