#ifndef hep_hpc_Utilities_detail_index_sequence_hpp
#define hep_hpc_Utilities_detail_index_sequence_hpp

#include "hep_hpc/detail/config.hpp"

#ifdef HEP_HPC_USE_BOOST_INDEX_SEQUENCE
#include <boost/fusion/support/detail/index_sequence.hpp>
#else
#include <utility>
#endif

namespace hep_hpc {
  namespace detail {
#ifdef HEP_HPC_USE_BOOST_INDEX_SEQUENCE
    using boost::fusion::detail::index_sequence;
    using boost::fusion::detail::make_index_sequence;
#else
    using std::index_sequence;
    using std::make_index_sequence;
#endif
  }
}
#endif /* hep_hpc_Utilities_detail_index_sequence_hpp */
