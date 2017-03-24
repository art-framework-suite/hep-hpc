#ifndef hep_hpc_H5File_hpp
#define hep_hpc_H5File_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::H5File
//
// Simple (not yet full-featured) class managing an HDF5 file resource.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/SimpleRAII.hpp"
#include "hep_hpc/HID_t.hpp"

#include "hdf5.h"

namespace hep_hpc {
  class H5File;
}

class hep_hpc::H5File {
public:
  H5File() = default;

  explicit H5File(std::string const & filename,
                  unsigned int flags = H5F_ACC_TRUNC,
                  hid_t fcpl_id = H5P_DEFAULT,
                  hid_t fapl_id = H5P_DEFAULT);

  operator hid_t() const noexcept;

  explicit operator bool () const noexcept;

  // Explicitly close the file.
  void close();

private:
  SimpleRAII<HID_t> h5file_;
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
  static const HID_t INVALID_FILE;
  return *h5file_ != INVALID_FILE;
}

inline
void
hep_hpc::H5File::
close() {
  h5file_.reset();
}
#endif /* hep_hpc_H5File_hpp */
