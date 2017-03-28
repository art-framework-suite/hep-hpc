#ifndef hep_hpc_H5Dataset_hpp
#define hep_hpc_H5Dataset_hpp

#include "hep_hpc/HID_t.hpp"
#include "hep_hpc/H5Dataspace.hpp"
#include "hep_hpc/H5PropertyList.hpp"
#include "hep_hpc/SimpleRAII.hpp"

#include "hdf5.h"

namespace hep_hpc {
  class H5Dataset;
}

class hep_hpc::H5Dataset {
public:
  H5Dataset() = default;

  // Create.
  H5Dataset(hid_t fileOrGroup,
            std::string const & fullPathName,
            hid_t dtype,
            H5Dataspace dspace = H5Dataspace {H5S_SCALAR},
            H5PropertyList linkCreationProperties = {},
            H5PropertyList datasetCreationProperties = {},
            H5PropertyList datasetAccessProperties = {});

  // Open.
  H5Dataset(hid_t fileOrGroup,
            std::string const & fullPathName,
            H5PropertyList datasetAccessProperties = {});

  // Default move operations.
  H5Dataset(H5Dataset &&) = default;
  H5Dataset & operator = (H5Dataset &&) = default;

  operator hid_t() const noexcept;

  explicit operator bool () const noexcept;

  // Write.
  herr_t write(hid_t memType, void const * buf,
              H5Dataspace memSpace = {},
              H5Dataspace fileSpace = {},
              H5PropertyList transferProperties = {});

  // Read.
  herr_t read(hid_t memType, void * buf,
              H5Dataspace memSpace = {},
              H5Dataspace fileSpace = {},
              H5PropertyList transferProperties = {});

  // Flush.
  herr_t flush();

  // Refresh.
  herr_t refresh();

  // Reset (not usually required).
  void reset();

private:
  static hep_hpc::HID_t const INVALID_DSET_;
  SimpleRAII<HID_t> h5dset_;
};
#endif /* hep_hpc_H5Dataset_hpp */

inline
hep_hpc::H5Dataset::
operator hid_t() const noexcept
{
  return *h5dset_;
}

inline
hep_hpc::H5Dataset::
operator bool () const noexcept
{
  return *h5dset_ > INVALID_DSET_;
}

inline
herr_t
hep_hpc::H5Dataset::
write(hid_t const memType, void const * const buf,
      H5Dataspace const memSpace,
      H5Dataspace const fileSpace,
      H5PropertyList const transferProperties)
{
  return H5Dwrite(*h5dset_, memType,
                  memSpace, fileSpace, transferProperties, buf);
}

inline
herr_t
hep_hpc::H5Dataset::
read(hid_t const memType, void * const buf,
     H5Dataspace const memSpace,
     H5Dataspace const fileSpace,
     H5PropertyList const transferProperties)
{
  return H5Dread(*h5dset_, memType,
                 memSpace, fileSpace, transferProperties, buf);
}

inline
herr_t
hep_hpc::H5Dataset::
flush()
{
  return H5Dflush(*h5dset_);
}

inline
herr_t
hep_hpc::H5Dataset::
refresh()
{
  return H5Drefresh(*h5dset_);
}

inline
void
hep_hpc::H5Dataset::
reset() {
  h5dset_.reset();
}
