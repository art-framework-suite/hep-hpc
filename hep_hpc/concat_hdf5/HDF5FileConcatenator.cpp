#include "hep_hpc/concat_hdf5/HDF5FileConcatenator.hpp"

#include "hep_hpc/concat_hdf5/maybe_report_rank.hpp"
#include "hep_hpc/hdf5/Dataset.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/ResourceStrategy.hpp"
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
    return (stat(file_name.c_str(), &buffer) == 0);
  }
}

hep_hpc::HDF5FileConcatenator::
HDF5FileConcatenator(std::string const & output,
                     unsigned int file_mode,
                     std::size_t mem_max,
                     std::string filename_column [[gnu::unused]], // FIXME
                     std::vector<std::string> const & only_groups [[gnu::unused]], // FIXME
                     bool want_filters,
                     bool want_collective_writes,
                     int verbosity)
  : mem_max_bytes_(mem_max * 1024 * 1024)
  , want_filters_(want_filters)
  , want_collective_writes_(want_collective_writes)
  , verbosity_(verbosity)
  , h5out_(open_output_file_(output, file_mode))
{
}

int
hep_hpc::HDF5FileConcatenator::
concatFiles(std::vector<std::string> const & inputs [[gnu::unused]])
{
  // FIXME Need to treat filename_column here.

  // Iterate over files:
  for (auto const & input_file_name : inputs) {
    // 1. Open input file
    report_(3, std::string("Attempting to open input file ") + input_file_name);
    hdf5::File input_file(input_file_name, H5F_ACC_RDONLY, {}, maybe_collective_access_());

    // 2. Discover and iterate over items.
    report_(0, std::string("Processing input file ") + input_file_name);
    (void) hdf5::ErrorController::
      call(&H5Ovisit, input_file,
           H5_INDEX_NAME,
           H5_ITER_NATIVE,
           [](hid_t id,
              char const * name,
              H5O_info_t const * info,
              void * me) -> herr_t {
             return reinterpret_cast<HDF5FileConcatenator *>(me)->
               visit_item_(id, name, info);
           },
           this);

    // FIXME: fill filename_column here.
  }
  report_(3, "Completed processing input files");
  return 0;
}

void
hep_hpc::HDF5FileConcatenator::
report_(int level, std::string const & msg) const
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

hep_hpc::hdf5::PropertyList
hep_hpc::HDF5FileConcatenator::
maybe_collective_access_() const
{
  hdf5::PropertyList file_access_properties(H5P_FILE_ACCESS);
#ifdef HEP_HPC_USE_MPI
  report_(0, "Setting output file access properties to use MPI I/O.");
  (void) hdf5::ErrorController::call(&H5Pset_fapl_mpio,
                                     file_access_properties,
                                     MPI_COMM_WORLD,
                                     MPI_INFO_NULL);
#endif
  return file_access_properties;
}

hep_hpc::hdf5::File
hep_hpc::HDF5FileConcatenator::
open_output_file_(std::string file_name,
                  unsigned int file_mode) const
{
  // We want a slightly different semantic for "append" (H5F_ACC_RDWR)
  // than that provided by HDF5: if it exists, open it read/write; if it
  // does not, create it.
  if (file_mode == H5F_ACC_RDWR &&  ! file_exists(file_name)) {
    file_mode = H5F_ACC_EXCL;
  }
  hdf5::File output_file(file_name, file_mode, {}, maybe_collective_access_());
  return output_file;
}

herr_t
hep_hpc::HDF5FileConcatenator::
visit_item_(hid_t id,
            char const * name,
            H5O_info_t const * info)
{
  using std::to_string;
  herr_t status = 0;
  // FIXME: handle only_groups.
  switch (info->type) {
  case H5O_TYPE_GROUP:
    report_(2, std::string("Ensuring existence of group ") + name + " in output file.");
    // FIXME Ensure group's existence.
    break;
  case H5O_TYPE_DATASET:
    status = handle_dataset_(name, id);
    break;
  case H5O_TYPE_NAMED_DATATYPE:
    report_(-1, std::string("Ignoring named datatype ") + name);
    break;
  default:
    report_(-2, std::string("Unrecognized HDF5 object type ") +
            to_string(info->type));
    status = -1;
  }
  return status;
}

herr_t
hep_hpc::HDF5FileConcatenator::
handle_dataset_(const char * name [[gnu::unused]],
                hid_t id [[gnu::unused]])
{
  herr_t status = 0;
  hdf5::Dataset ds_in(id, hdf5::ResourceStrategy::observer_tag);

  // 1. Discover incoming dataset shape and size.
  report_(2, std::string("Examining shape and size for input dataset ") + name);

  // 2. Check if dataset exists in output. Create and store datasets and
  //    associated information in class state.

  // 3. Resize the dataset for the entire input file.

  // 4. Iterate over buffer-sized chunks.

  // 4.1 Calculate row numerology.

  // 4.2 Read the correct hyperslab of the input file and copy it to the
  //     output.

  return status;
}
