#ifndef hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp
#define hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp

#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/detail/config.hpp"

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
                       bool want_collective,
                       int verbosity);

  int concatFiles(std::vector<std::string> const & inputs);

private:
  void report_(int level, std::string const & msg);
  hdf5::File open_output_file_(std::string file_name,
                               unsigned int file_mode,
                               bool want_collective);

  // Parameters.
  std::size_t mem_max_bytes_;
  bool want_filters_;
  bool want_collective_;
  int verbosity_;

  // Other state.
  hdf5::File h5out_;
};

#endif /* hep_hpc_concat_hdf5_HDF5FileConcatenator_hpp */
