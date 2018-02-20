#ifndef hep_hpc_concat_hdf5_ConcatenatedDSInfo_hpp
#define hep_hpc_concat_hdf5_ConcatenatedDSInfo_hpp
////////////////////////////////////////////////////////////////////////
// ConcatenatedDSInfo
//
// Concatenation state information for a dataset.
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/hdf5/Dataset.hpp"

#include <vector>

namespace hep_hpc {
  struct ConcatenatedDSInfo;
}

struct hep_hpc::ConcatenatedDSInfo {
  std::size_t row_size_bytes {0ull};
  hsize_t chunk_rows {0ul};
  hsize_t buffer_size_rows {0ull};
  hdf5::Dataset ds;
  hsize_t n_rows_written_total {0ull};
};

#endif /* hep_hpc_concat_hdf5_ConcatenatedDSInfo_hpp */
