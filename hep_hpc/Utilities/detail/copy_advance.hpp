#ifndef hep_hpc_Utilities_detail_copy_advance_hpp
#define hep_hpc_Utilities_detail_copy_advance_hpp
////////////////////////////////////////////////////////////////////////
// copy_advance()
//
// Return an iterator advanced by n from the provided iterator i.
////////////////////////////////////////////////////////////////////////
#include <iterator>

namespace hep_hpc {
  namespace detail {
    template <typename IN_ITER, typename DISTANCE>
    IN_ITER copy_advance(IN_ITER i, DISTANCE const n)
    {
      std::advance(i, n);
      return i;
    }
  }
}
#endif /* hep_hpc_Utilities_detail_copy_advance_hpp */
