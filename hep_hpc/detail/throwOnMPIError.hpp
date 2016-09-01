#ifndef HDFSTUDY_DETAIL_THROWONMPIERROR_HPP
#define HDFSTUDY_DETAIL_THROWONMPIERROR_HPP
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

#endif /* HDFSTUDY_DETAIL_THROWONMPIERROR_HPP */
