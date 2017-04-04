#ifndef hep_hpc_hdf5_Dataset_hpp
#define hep_hpc_hdf5_Dataset_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::Dataset
//
// Class representing an HDF5 dataset, with resource management.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/hdf5/HID_t.hpp"
#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/Utilities/SimpleRAII.hpp"

#include "hdf5.h"

namespace hep_hpc {
  namespace hdf5 {
    class Dataset;
  }
}

class hep_hpc::hdf5::Dataset {
public:
  Dataset() = default;

  // Create.
  Dataset(hid_t fileOrGroup,
            std::string const & fullPathName,
            hid_t dtype,
            Dataspace dspace = Dataspace {H5S_SCALAR},
            PropertyList linkCreationProperties = {},
            PropertyList datasetCreationProperties = {},
            PropertyList datasetAccessProperties = {});

  // Open.
  Dataset(hid_t fileOrGroup,
            std::string const & fullPathName,
            PropertyList datasetAccessProperties = {});

  // Default move operations.
  Dataset(Dataset &&) = default;
  Dataset & operator = (Dataset &&) = default;

  operator hid_t() const noexcept;

  explicit operator bool () const noexcept;

  // Write.
  herr_t write(hid_t memType, void const * buf,
              Dataspace memSpace = {},
              Dataspace fileSpace = {},
              PropertyList transferProperties = {});

  // Read.
  herr_t read(hid_t memType, void * buf,
              Dataspace memSpace = {},
              Dataspace fileSpace = {},
              PropertyList transferProperties = {});

  // Flush.
  herr_t flush();

  // Refresh.
  herr_t refresh();

  // Reset (not usually required).
  void reset();

private:
  static HID_t const INVALID_DSET_;
  detail::SimpleRAII<HID_t> h5dset_;
};
#endif /* hep_hpc_hdf5_Dataset_hpp */

inline
hep_hpc::hdf5::Dataset::
operator hid_t() const noexcept
{
  return *h5dset_;
}

inline
hep_hpc::hdf5::Dataset::
operator bool () const noexcept
{
  return *h5dset_ > INVALID_DSET_;
}

inline
herr_t
hep_hpc::hdf5::Dataset::
write(hid_t const memType, void const * const buf,
      Dataspace const memSpace,
      Dataspace const fileSpace,
      PropertyList const transferProperties)
{
  return H5Dwrite(*h5dset_, memType,
                  memSpace, fileSpace, transferProperties, buf);
}

inline
herr_t
hep_hpc::hdf5::Dataset::
read(hid_t const memType, void * const buf,
     Dataspace const memSpace,
     Dataspace const fileSpace,
     PropertyList const transferProperties)
{
  return H5Dread(*h5dset_, memType,
                 memSpace, fileSpace, transferProperties, buf);
}

inline
herr_t
hep_hpc::hdf5::Dataset::
flush()
{
  return H5Dflush(*h5dset_);
}

inline
herr_t
hep_hpc::hdf5::Dataset::
refresh()
{
  return H5Drefresh(*h5dset_);
}

inline
void
hep_hpc::hdf5::Dataset::
reset() {
  h5dset_.reset();
}
