#ifndef HDFSTUDY_THROWONMPIERROR_HPP
#define HDFSTUDY_THROWONMPIERROR_HPP
////////////////////////////////////////////////////////////////////////
// hep_hpc::throwOnMPIError()
//
// Throw an exception upon non-successful return from an MPI function.
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/detail/demangle_symbol.hpp"
#include "hep_hpc/detail/throwOnMPIError.hpp"

#include <string>
#include <type_traits>
#include <typeinfo>

namespace hep_hpc {
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
