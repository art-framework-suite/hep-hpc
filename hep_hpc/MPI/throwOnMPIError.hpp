#ifndef hep_hpc_MPI_throwOnMPIError_hpp
#define hep_hpc_MPI_throwOnMPIError_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::throwOnMPIError
//
// Execute the provided MPI function (with arguments if provided) and
// throw an exception on failure.
//
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/MPI/detail/throwMPIError.hpp"

#include <string>

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
    detail::throwMPIError(funcString, status);
  }
}

#endif /* hep_hpc_MPI_throwOnMPIError_hpp */
