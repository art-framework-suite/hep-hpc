#ifndef hep_hpc_hdf5_File_hpp
#define hep_hpc_hdf5_File_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::File
//
// Simple class managing an HDF5 file resource.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/ResourceStrategy.hpp"
#include "hep_hpc/hdf5/HID_t.hpp"
#include "hep_hpc/hdf5/Resource.hpp"

#include "hdf5.h"

namespace hep_hpc {
  namespace hdf5 {
    class File;
  }
}

class hep_hpc::hdf5::File {
public:
  File() = default;

  // Adopt an existing HDF5 file ID, with the specified management
  // strategy. If observing (default), caller is responsible for closing
  // the file at the appropriate time.
  explicit File(hid_t file,
                ResourceStrategy strategy = ResourceStrategy::observer_tag);

  // Open or create an HDF5 file, as appropriate. Property lists may be
  // provided with move semantics to avoid a copy if desired.
  explicit File(std::string const & filename,
                unsigned int flag = H5F_ACC_RDONLY,
                PropertyList fileCreationProperties = {},
                PropertyList fileAccessProperties = {});

  operator hid_t() const noexcept;

  explicit operator bool () const noexcept;

  // Flush the file contents.
  herr_t flush(H5F_scope_t scope = H5F_SCOPE_GLOBAL);

  // Explicitly close the file.
  void close();

private:
  Resource h5file_;
};

inline
hep_hpc::hdf5::File::File(hid_t const file,
                          ResourceStrategy const strategy)
  :
  h5file_((strategy == ResourceStrategy::handle_tag) ?
          Resource(file, &H5Fclose) :
          Resource(file))
{
}

inline
hep_hpc::hdf5::File::
operator hid_t() const noexcept
{
  return *h5file_;
}

inline
hep_hpc::hdf5::File::
operator bool () const noexcept
{
  static const HID_t INVALID_FILE;
  return *h5file_ > INVALID_FILE;
}

inline
herr_t
hep_hpc::hdf5::File::
flush(H5F_scope_t const scope)
{
  return ErrorController::call(&H5Fflush, *h5file_, scope);
}

inline
void
hep_hpc::hdf5::File::
close() {
  h5file_.reset();
}

#endif /* hep_hpc_hdf5_File_hpp */
