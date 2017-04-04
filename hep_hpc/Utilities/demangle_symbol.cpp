// ======================================================================
//
// demangle: Call the cross-vendor API to demangle symbols
//           (eg for ROOT dictionary errors).
//
// ======================================================================

#include "hep_hpc/Utilities/demangle_symbol.hpp"

#include <cstdlib>
#include <cxxabi.h>

namespace {
  struct buffer_sentry {
    buffer_sentry() : buf(0) { }
    ~buffer_sentry() noexcept { free(buf); }

    char * buf;
  };  // buffer_sentry
}

std::string
hep_hpc::detail::
demangle_symbol(std::string const &mangled)
{
  buffer_sentry unmangled; // Auto-free of buffer on exit from function
  size_t length;
  int status;
  unmangled.buf = abi::__cxa_demangle(mangled.c_str(),
                                      unmangled.buf,
                                      &length,
                                      &status);
  if (unmangled.buf == NULL) {
    return mangled; // Failure
  } else {
    return unmangled.buf;
  }
}
