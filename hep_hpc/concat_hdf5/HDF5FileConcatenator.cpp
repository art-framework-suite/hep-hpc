#include "hep_hpc/concat_hdf5/HDF5FileConcatenator.hpp"

#include "hep_hpc/concat_hdf5/maybe_report_rank.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

extern "C" {
#include "sys/stat.h"
#include "H5FDmpio.h"
}

#include <iostream>

namespace {
  bool file_exists(std::string file_name)
  {
    struct stat buffer;
    return (stat (file_name.c_str(), &buffer) == 0);
  }
}

hep_hpc::HDF5FileConcatenator::
HDF5FileConcatenator(std::string const & output,
                     unsigned int file_mode,
                     std::size_t mem_max,
                     std::string filename_column [[gnu::unused]],
                     std::vector<std::string> const & only_groups [[gnu::unused]],
                     bool want_filters,
                     bool want_collective,
                     int verbosity)
  : mem_max_bytes_(mem_max * 1024 * 1024)
  , want_filters_(want_filters)
  , want_collective_(want_collective)
  , verbosity_(verbosity)
  , h5out_(open_output_file_(output, file_mode, want_collective_))
{
}

int
hep_hpc::HDF5FileConcatenator::
concatFiles(std::vector<std::string> const & inputs [[gnu::unused]])
{
  return 0;
}

void
hep_hpc::HDF5FileConcatenator::
report_(int level, std::string const & msg)
{
  using std::to_string;
  if (verbosity_ < level) {
    return;
  }
  std::string level_txt;
  switch (level) {
  case -2:
    level_txt = "ERROR";
    break;
  case -1:
    level_txt = "WARNING";
    break;
  case 0:
    level_txt = "INFO";
    break;
  default:
    level_txt = std::string("DEBUG(") + to_string(level) + ')';
  }
  std::ostream & os = (level > -1) ? std::cout : std::cerr;
  os << maybe_report_rank
     << level_txt
     << ": "
     << msg
     << std::endl;
}

hep_hpc::hdf5::File
hep_hpc::HDF5FileConcatenator::
open_output_file_(std::string file_name,
                  unsigned int file_mode,
                  bool want_collective)
{
  // We want a slightly different semantic for "append" (H5F_ACC_RDWR)
  // than that provided by HDF5: if it exists, open it read/write; if it
  // does not, create it.
  if (file_mode == H5F_ACC_RDWR &&  ! file_exists(file_name)) {
    file_mode = H5F_ACC_EXCL;
  }
  hdf5::PropertyList file_access_properties(H5P_FILE_ACCESS);
#ifdef HEP_HPC_USE_MPI
  // Active MPI I/O.
  if (want_collective) {
    report_(0, "Setting output file access properties to use MPI I/O.");
    (void) hdf5::ErrorController::call(&H5Pset_fapl_mpio,
                                       file_access_properties,
                                       MPI_COMM_WORLD,
                                       MPI_INFO_NULL);
  }
#endif
  hdf5::File output_file(file_name, file_mode, {},
                         std::move(file_access_properties));
  return output_file;
}
