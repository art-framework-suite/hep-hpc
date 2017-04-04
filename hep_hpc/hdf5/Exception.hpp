#ifndef hep_hpc_hdf5_Exception_hpp
#define hep_hpc_hdf5_Exception_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::Exception
//
// Simple exception class to distinguish HDF5-related exceptions.
//
////////////////////////////////////////////////////////////////////////

#include <stdexcept>
#include <string>

namespace hep_hpc {
  namespace hdf5 {
    class Exception;
  }
}

class hep_hpc::hdf5::Exception : public std::runtime_error {
public:
  // Use base class' constructors.
  using std::runtime_error::runtime_error;
};
#endif /* hep_hpc_hdf5_Exception_hpp */
