#ifndef hep_hpc_detail_throwOnMPIError_hpp
#define hep_hpc_detail_throwOnMPIError_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::detail::throwOnMPIError()
//
// Throw an exception with the provided message on non-zero status.
//
////////////////////////////////////////////////////////////////////////
#include <string>

namespace hep_hpc {
  namespace detail {
    void throwOnMPIError(std::string const & funcString, int status);
  }
}

#endif /* hep_hpc_detail_throwOnMPIError_hpp */
