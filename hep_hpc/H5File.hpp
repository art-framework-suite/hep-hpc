#ifndef HDFSTUDY_H5FILE_HPP
#define HDFSTUDY_H5FILE_HPP
////////////////////////////////////////////////////////////////////////
// hep_hpc::H5File
//
// Simple (not yet full-featured) class managing an HDF5 file resource.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/SimpleRAII.hpp"

#include "hdf5.h"

namespace hep_hpc {
  class H5File;
}

class hep_hpc::H5File {
public:
  H5File(std::string const & filename,
         unsigned int flags = H5F_ACC_TRUNC);

private:
  SimpleRAII<hid_t> h5file_;
};
#endif /* HDFSTUDY_H5FILE_HPP */
