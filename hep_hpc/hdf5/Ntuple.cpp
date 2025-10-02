#include "hep_hpc/hdf5/Ntuple.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include <stdexcept>

hep_hpc::hdf5::File
hep_hpc::hdf5::NtupleDetail::verifiedFile(File file)
{
  if (!file) {
    throw std::runtime_error("Attempt to create Ntuple with invalid File.");
  }
  unsigned intent;
  auto istat = ErrorController::call(&H5Fget_intent, file, &intent);
  if (istat != (herr_t)0) {
    throw std::runtime_error("Error obtaining file mode.");
  } 
  if (intent == H5F_ACC_RDONLY 
#if H5_VERS_MAJOR > 1 || (H5_VERS_MAJOR == 1 && H5_VERS_MINOR >= 10)
    || intent == H5F_ACC_SWMR_READ
#endif
    ) {
    throw std::runtime_error("Reading from HDF5 Ntuple file not supported!");
  }
  return file;
}

hep_hpc::hdf5::PropertyList
hep_hpc::hdf5::NtupleDetail::fileAccessProperties()
{
  // Ensure we are using the latest available HDF5 file format to
  // write our data.
  PropertyList plist(H5P_FILE_ACCESS);
  H5Pset_libver_bounds(plist, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  return plist;
}
