#ifndef hep_hpc_H5File_hpp
#define hep_hpc_H5File_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::H5File
//
// Simple (not yet full-featured) class managing an HDF5 file resource.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/SimpleRAII.hpp"
#include "hep_hpc/detail/DefaultedSimpleType.hpp"
#include "hdf5.h"

namespace hep_hpc {
  class H5File;
}

class hep_hpc::H5File {
public:
  H5File() = default;

  H5File(std::string const & filename,
         unsigned int flags = H5F_ACC_TRUNC);

  operator hid_t() const noexcept;

  explicit operator bool () const noexcept;

private:
  static constexpr const hid_t INVALID_FILE = -1;
  SimpleRAII<detail::DefaultedSimpleType<hid_t, INVALID_FILE>> h5file_;
};

inline
hep_hpc::H5File::
operator hid_t() const noexcept
{
  return *h5file_;
}

inline
hep_hpc::H5File::
operator bool () const noexcept
{
  return *h5file_ != INVALID_FILE;
}
#endif /* hep_hpc_H5File_hpp */
