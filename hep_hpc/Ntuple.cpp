#include "hep_hpc/Ntuple.hpp"

hep_hpc::H5File
hep_hpc::NtupleDetail::verifiedFile(H5File file)
{
  if (!file) {
    throw std::runtime_error("Attempt to create Ntuple with invalid H5File.");
  }
  unsigned intent;
  auto istat = H5Fget_intent(file, &intent);
  if (istat != (herr_t)0) {
    throw std::runtime_error("Error obtaining file mode.");
  } 
  if (intent == H5F_ACC_RDONLY || intent == H5F_ACC_SWMR_READ) {
    throw std::runtime_error("Reading from HDF5 Ntuple file not supported!");
  }
  return file;
}
