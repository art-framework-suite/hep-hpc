#ifndef hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp
#define hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp

#include "hep_hpc/Utilities/detail/compiler_macros.hpp"
#include "hep_hpc/concat_hdf5/ConcatenatedDSInfo.hpp"
#include "hep_hpc/concat_hdf5/FilenameColumnInfo.hpp"
#include "hep_hpc/hdf5/Dataset.hpp"
#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/detail/config.hpp"

extern "C" {
#include "hdf5.h"
}

#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

namespace hep_hpc {
  class HDF5FileConcatenator;
}

class hep_hpc::HDF5FileConcatenator {
public:
  HDF5FileConcatenator(std::string const & output,
                       unsigned int file_mode,
                       long long max_rows,
                       std::size_t mem_max_bytes,
                       FilenameColumnInfo filename_column_info,
                       std::vector<std::regex> const & only_groups,
                       bool want_filters,
                       bool force_compression,
                       bool want_collective_writes,
                       int verbosity);

  int concatFiles(std::vector<std::string> const & inputs);

private:
  // Visitor callback for use by H5Ovisit().
  herr_t visit_item_(hid_t root_id,
                     char const * obj_name,
                     H5O_info_t const * obj_info);
  // Handle the data movement for one dataset.
  herr_t handle_dataset_(hdf5::Dataset ds_in, std::string ds_name);
  // Return a suitably-set property list specifying properties for read
  // and write.
  hdf5::PropertyList transfer_properties_();

  // Parameters.
  long long max_rows_;
  hsize_t mem_max_bytes_;
  bool want_filters_;
  bool force_compression_;
  bool want_collective_writes_
#ifndef HEP_HPC_USE_MPI
  // Satisfy picky compilers if we're not compiled with MPI.
  UNUSED_PRIVATE_FIELD
#endif
  ;
  FilenameColumnInfo filename_column_info_;
  std::vector<std::string> filename_column_data_;
  std::vector<std::regex> only_groups_;

  // I/O buffer.
  std::vector<uint8_t> buffer_;

  // N.B. Relative order of h5out_ and ds_info_ should result in output
  // datasets being closed before the output file.
  // Output file.
  hdf5::File h5out_;
  // Per-dataset info and state.
  std::unordered_map<std::string, ConcatenatedDSInfo> ds_info_;
  // Per-group info.
  struct FilenameColumnDSInfo {
    std::string name;
    hdf5::Dataset ds;
    hsize_t current_size { 0ull };
    hsize_t required_size { 0ull };
  };
  std::unordered_map<std::string, FilenameColumnDSInfo>
  group_filename_column_ds_size_;
};

#endif /* hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp */
