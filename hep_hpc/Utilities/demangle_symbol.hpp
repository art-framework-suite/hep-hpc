#ifndef hep_hpc_Utilities_demangle_symbol_hpp
#define hep_hpc_Utilities_demangle_symbol_hpp

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

#endif /* hep_hpc_Utilities_demangle_symbol_hpp */
