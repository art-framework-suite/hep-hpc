#ifndef hep_hpc_concat_hdf5_ConcatenatedDSInfo_hpp
#define hep_hpc_concat_hdf5_ConcatenatedDSInfo_hpp
////////////////////////////////////////////////////////////////////////
// ConcatenatedDSInfo
//
// Hang on to concatenation state information for a dataset.
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/hdf5/Dataset.hpp"

#include <vector>

namespace hep_hpc {
  struct ConcatenatedDSInfo;
}

struct hep_hpc::ConcatenatedDSInfo {
  std::size_t ndims() const { return current_shape.size(); }
  hdf5::Dataset ds;
  std::size_t row_size_bytes {0ull};
  std::size_t buffer_size_rows {0ull};
  hsize_t chunk_rows {0ul};
  std::size_t rows_left_this_chunk {0ull};
  std::vector<hsize_t> current_shape;
};

#endif /* hep_hpc_concat_hdf5_ConcatenatedDSInfo_hpp */
