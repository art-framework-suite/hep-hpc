#ifndef hep_hpc_H5Exception_hpp
#define hep_hpc_H5Exception_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::H5Exception
//
// Simple exception class to distinguish HDF5-related exceptions.
//
////////////////////////////////////////////////////////////////////////

#include <stdexcept>
#include <string>

namespace hep_hpc {
  class H5Exception;
}

class hep_hpc::H5Exception : public std::runtime_error {
public:
  // Use base class' constructors.
  using std::runtime_error::runtime_error;
};
#endif /* hep_hpc_H5Exception_hpp */
