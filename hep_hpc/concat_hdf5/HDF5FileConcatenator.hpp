#ifndef hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp
#define hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp

#include "hep_hpc/concat_hdf5/ConcatenatedDSInfo.hpp"
#include "hep_hpc/hdf5/Dataset.hpp"
#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/detail/config.hpp"

extern "C" {
#include "hdf5.h"
}

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
                       std::size_t mem_max,
                       std::string filename_column,
                       std::vector<std::string> const & only_groups,
                       bool want_filters,
                       bool want_collective_writes,
                       int verbosity);

  int concatFiles(std::vector<std::string> const & inputs);

private:
  herr_t visit_item_(hid_t root_id,
                     char const * obj_name,
                     H5O_info_t const * obj_info);
  herr_t handle_dataset_(hdf5::Dataset ds_in, const char * ds_name);

  // Parameters.
  std::size_t mem_max_bytes_;
  bool want_filters_;
  bool want_collective_writes_;

  // Other state.

  // N.B. Relative order of h5out_ and ds_info_ should result in output
  // datasets being closed before the output file.
  hdf5::File h5out_;
  std::unordered_map<std::string, ConcatenatedDSInfo> ds_info_;
};

#endif /* hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp */
