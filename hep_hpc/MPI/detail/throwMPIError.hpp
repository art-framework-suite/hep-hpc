#ifndef hep_hpc_MPI_detail_throwMPIError_hpp
#define hep_hpc_MPI_detail_throwMPIError_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::detail::throwMPIError
//
// Unconditionally throw an exception expanding on MPI error information
// about the specified error.
//
////////////////////////////////////////////////////////////////////////
#include <string>

namespace hep_hpc {
  namespace detail {
    void throwMPIError(std::string const & funcString, int status);
  }
}
#endif /* hep_hpc_MPI_detail_throwMPIError_hpp */
