#ifndef hep_hpc_H5File_hpp
#define hep_hpc_H5File_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::H5File
//
// Simple (not yet full-featured) class managing an HDF5 file resource.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/H5PropertyList.hpp"
#include "hep_hpc/HID_t.hpp"
#include "hep_hpc/SimpleRAII.hpp"

#include "hdf5.h"

namespace hep_hpc {
  class H5File;
}

class hep_hpc::H5File {
public:
  H5File() = default;

  // Non-owning.
  //
  // Caller is responsible for ensuring file is a valid HDF5 file
  // handle, and for ensuring that it is closed afterwards.
  explicit H5File(hid_t file);

  // Open or create an HDF5 file, as appropriate. Note that the
  // H5PropertyList objects (if specified) may be moved in or copied by
  // value, but anyway do not need to live beyond this call.
  explicit H5File(std::string filename,
                  unsigned int flag = H5F_ACC_RDONLY,
                  H5PropertyList fileCreationProperties = {},
                  H5PropertyList fileAccessProperties = {});

  operator hid_t() const noexcept;

  explicit operator bool () const noexcept;

  // Flush the file contents.
  herr_t flush(H5F_scope_t scope = H5F_SCOPE_GLOBAL);

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
  return *h5file_ > INVALID_FILE;
}

inline
herr_t
hep_hpc::H5File::
flush(H5F_scope_t const scope)
{
  return H5Fflush(*h5file_, scope);
}

inline
void
hep_hpc::H5File::
close() {
  h5file_.reset();
}
#endif /* hep_hpc_H5File_hpp */
