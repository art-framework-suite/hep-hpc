#ifndef hep_hpc_Utilities_is_nothrow_swappable_all_hpp
#define hep_hpc_Utilities_is_nothrow_swappable_all_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::detail::is_nothrow_swappable_all
//
// Class template type trait to determine whether the swap operations
// for a series of types are all noexcept.
//
// See
// http://cpptruths.blogspot.com/2011/09/tale-of-noexcept-swap-for-user-defined.html
// for a discussion and the below code.
//
////////////////////////////////////////////////////////////////////////
#include <tuple>

namespace hep_hpc {
  template<typename... T>
  struct is_nothrow_swappable_all
  {
    static constexpr std::tuple<T...> *t = nullptr;
    enum { value = noexcept(t->swap(*t)) };
  };
}
#endif /* hep_hpc_Utilities_is_nothrow_swappable_all_hpp */
