#ifndef HDFSTUDY_DEMANGLE_SYMBOL_HPP
#define HDFSTUDY_DEMANGLE_SYMBOL_HPP

// ======================================================================
//
// demangle: Call the cross-vendor API to demangle symbols
//           (eg for ROOT dictionary errors).
//
// ======================================================================

#include <string>

namespace hep_hpc {
  namespace detail {
    std::string demangle_symbol(std::string const &mangled);
  }
}

#endif /* HDFSTUDY_DEMANGLE_SYMBOL_HPP */
