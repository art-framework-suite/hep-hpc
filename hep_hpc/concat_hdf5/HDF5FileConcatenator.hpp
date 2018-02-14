#ifndef hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp
#define hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp

#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/detail/config.hpp"

extern "C" {
#include "hdf5.h"
}

#include <string>
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
  void report_(int level, std::string const & msg) const;
  hdf5::PropertyList maybe_collective_access_() const;
  hdf5::File open_output_file_(std::string file_name,
                               unsigned int file_mode) const;
  herr_t visit_item_(hid_t id,
                     char const * name,
                     H5O_info_t const * info);

  herr_t handle_dataset_(const char * name [[gnu::unused]],
                         hid_t id [[gnu::unused]]);

  // Parameters.
  std::size_t mem_max_bytes_;
  bool want_filters_;
  bool want_collective_writes_;
  int verbosity_;

  // Other state.
  hdf5::File h5out_;
};

#endif /* hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp */
