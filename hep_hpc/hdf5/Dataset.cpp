#include "hep_hpc/hdf5/Dataset.hpp"

hep_hpc::hdf5::Dataset::
Dataset(hid_t const fileOrGroup,
          std::string const & fullPathName,
          hid_t const dtype,
          Dataspace const dspace,
          PropertyList const linkCreationProperties,
          PropertyList const datasetCreationProperties,
          PropertyList const datasetAccessProperties)
  :
  h5dset_([&]()
          { return H5Dcreate2(fileOrGroup,
                              fullPathName.c_str(),
                              dtype,
                              dspace,
                              linkCreationProperties,
                              datasetCreationProperties,
                              datasetAccessProperties);
          }, &H5Dclose)
{
}

hep_hpc::hdf5::Dataset::
Dataset(hid_t const fileOrGroup,
          std::string const & fullPathName,
          PropertyList const datasetAccessProperties)
  :
  h5dset_([&]()
          { return H5Dopen2(fileOrGroup,
                            fullPathName.c_str(),
                            datasetAccessProperties);
          }, &H5Dclose)
{
}

hep_hpc::hdf5::HID_t const
hep_hpc::hdf5::Dataset::INVALID_DSET_;
