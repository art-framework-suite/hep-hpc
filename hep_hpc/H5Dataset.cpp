#include "hep_hpc/H5Dataset.hpp"

hep_hpc::H5Dataset::
H5Dataset(hid_t const fileOrGroup,
          std::string const & fullPathName,
          hid_t const dtype,
          H5Dataspace const dspace,
          H5PropertyList const linkCreationProperties,
          H5PropertyList const datasetCreationProperties,
          H5PropertyList const datasetAccessProperties)
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

hep_hpc::H5Dataset::
H5Dataset(hid_t const fileOrGroup,
          std::string const & fullPathName,
          H5PropertyList const datasetAccessProperties)
  :
  h5dset_([&]()
          { return H5Dopen2(fileOrGroup,
                            fullPathName.c_str(),
                            datasetAccessProperties);
          }, &H5Dclose)
{
}

hep_hpc::HID_t const
hep_hpc::H5Dataset::INVALID_DSET_;
