#ifndef hep_hpc_Utilities_demangle_symbol_hpp
#define hep_hpc_Utilities_demangle_symbol_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::detail::demangle_symbol()
//
// Call the cross-vendor API to demangle symbols.
//
////////////////////////////////////////////////////////////////////////

#include <string>
#include <typeinfo>

namespace hep_hpc {
  namespace detail {
    std::string demangle_symbol(std::string const &mangled);

    template <typename FUNC>
    std::string
    demangle_function(FUNC func)
    {
      return demangle_symbol(typeid(func).name());
    }
  }
}

#endif /* hep_hpc_Utilities_demangle_symbol_hpp */
