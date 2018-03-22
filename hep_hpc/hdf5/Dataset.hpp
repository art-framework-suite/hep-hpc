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
#include "hep_hpc/hdf5/Resource.hpp"
#include "hep_hpc/hdf5/ResourceStrategy.hpp"

#include "hdf5.h"

namespace hep_hpc {
  namespace hdf5 {
    class Dataset;
  }
}

class hep_hpc::hdf5::Dataset {
public:
  Dataset() = default;

  // Use an existing HDF5 dataset ID, and manage as specified.
  Dataset(hid_t dataset, ResourceStrategy strategy);

  // Create. Data spaces and property lists may be provided with move
  // semantics to avoid a copy if desired.
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

  operator hid_t() const noexcept;

  // Is this a valid dataset?
  bool is_valid() const noexcept;
  explicit operator bool () const noexcept;

  // Write. Data spaces and property lists may be provided with move
  // semantics to avoid a copy if desired.
  herr_t write(hid_t memType, void const * buf,
               Dataspace memSpace = {},
               Dataspace fileSpace = {},
               PropertyList transferProperties = {});

  // Read. Data spaces and property lists may be provided with move
  // semantics to avoid a copy if desired.
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
  static constexpr HID_t INVALID_DSET_() { return HID_t {}; }
  Resource h5dset_;
};

inline
hep_hpc::hdf5::Dataset::
Dataset(hid_t const dataset, ResourceStrategy const strategy)
  : h5dset_((strategy == ResourceStrategy::handle_tag) ?
            Resource(dataset, &H5Dclose) :
            Resource(dataset))
{
}

inline
hep_hpc::hdf5::Dataset::
Dataset(hid_t const fileOrGroup,
        std::string const & fullPathName,
        hid_t const dtype,
        Dataspace dspace,
        PropertyList linkCreationProperties,
        PropertyList datasetCreationProperties,
        PropertyList datasetAccessProperties)
  :
  h5dset_(&H5Dcreate2,
          &H5Dclose,
          fileOrGroup,
          fullPathName.c_str(),
          dtype,
          dspace,
          linkCreationProperties,
          datasetCreationProperties,
          datasetAccessProperties)
{
  if (*h5dset_ < 0) { // Error that we didn't throw over.
    h5dset_.release();
  }
}

inline
hep_hpc::hdf5::Dataset::
Dataset(hid_t const fileOrGroup,
        std::string const & fullPathName,
        PropertyList datasetAccessProperties)
  :
  h5dset_(&H5Dopen2, &H5Dclose,
          fileOrGroup,
          fullPathName.c_str(),
          datasetAccessProperties)
{
  if (*h5dset_ < 0) { // Error that we didn't throw over.
    h5dset_.release();
  }
}

inline
hep_hpc::hdf5::Dataset::
operator hid_t() const noexcept
{
  return *h5dset_;
}

inline
bool
hep_hpc::hdf5::Dataset::
is_valid () const noexcept
{
  return *h5dset_ > INVALID_DSET_();
}

inline
hep_hpc::hdf5::Dataset::
operator bool () const noexcept
{
  return is_valid();
}

inline
herr_t
hep_hpc::hdf5::Dataset::
write(hid_t const memType, void const * const buf,
      Dataspace memSpace,
      Dataspace fileSpace,
      PropertyList transferProperties)
{
  return ErrorController::call(&H5Dwrite,
                               *h5dset_,
                               memType,
                               memSpace,
                               fileSpace,
                               transferProperties,
                               buf);
}

inline
herr_t
hep_hpc::hdf5::Dataset::
read(hid_t const memType, void * const buf,
     Dataspace memSpace,
     Dataspace fileSpace,
     PropertyList transferProperties)
{
  return ErrorController::call(&H5Dread,
                               *h5dset_,
                               memType,
                               memSpace,
                               fileSpace,
                               transferProperties,
                               buf);
}

inline
herr_t
hep_hpc::hdf5::Dataset::
flush()
{
  return ErrorController::call(&H5Dflush, *h5dset_);
}

inline
herr_t
hep_hpc::hdf5::Dataset::
refresh()
{
  return ErrorController::call(&H5Drefresh, *h5dset_);
}

inline
void
hep_hpc::hdf5::Dataset::
reset() {
  h5dset_.reset();
}

#endif /* hep_hpc_hdf5_Dataset_hpp */
