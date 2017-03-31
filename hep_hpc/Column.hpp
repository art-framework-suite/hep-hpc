#ifndef hep_hpc_detail_column_hpp
#define hep_hpc_detail_column_hpp

#include "hdf5.h"

#include <array>
#include <string>
#include <utility>
#include <tuple>

namespace hep_hpc {
  template<typename T>
  struct Column;

}

////////////////////////////////////////////////////////////////////////
// No user-serviceable parts.
namespace hep_hpc {

  namespace detail {
    template <size_t N>
    using name_array = std::array<std::string, N>;

    class column_base {
  public:
      column_base(std::string const& n) : name_{n} {}
      auto const& name() const { return name_; }
  private:
      std::string name_;
    };
  }

  template <>
  struct Column<double> : detail::column_base {
    using column_base::column_base;
    using type = double;
    static hid_t engine_type() { return H5T_NATIVE_DOUBLE; }
  };

  template <>
  struct Column<float> : detail::column_base {
    using column_base::column_base;
    using type = float;
    static hid_t engine_type() { return H5T_NATIVE_FLOAT; }
  };

  template <>
  struct Column<int> : detail::column_base {
    using column_base::column_base;
    using type = int;
    static hid_t engine_type() { return H5T_NATIVE_INT; }
  };

  template <>
  struct Column<long> : detail::column_base {
    using column_base::column_base;
    using type = long;
    static hid_t engine_type() { return H5T_NATIVE_LONG; }
  };

  template <>
  struct Column<long long> : detail::column_base {
    using column_base::column_base;
    using type = long long;
    static hid_t engine_type() { return H5T_NATIVE_LLONG; }
  };

  template <>
  struct Column<unsigned int> : detail::column_base {
    using column_base::column_base;
    using type = unsigned int;
    static hid_t engine_type() { return H5T_NATIVE_UINT; }
  };

  template <>
  struct Column<unsigned long> : detail::column_base {
    using column_base::column_base;
    using type = unsigned long;
    static hid_t engine_type() { return H5T_NATIVE_ULONG; }
  };

  template <>
  struct Column<unsigned long long> : detail::column_base {
    using column_base::column_base;
    using type = unsigned long long;
    static hid_t engine_type() { return H5T_NATIVE_ULLONG; }
  };


  namespace detail {

    //=============================================================================
    // A permissive_column type is used in the context of an Ntuple so
    // that the following constructs are allowed:
    //
    //   Ntuple<int, double, string>   // has identical semantics to...
    //   Ntuple<column<int>, column<double>, column<string>>
    //
    // The benefit is that if a user wants to specify a constraint for
    // a given column, the way to do that in the context of the Ntuple
    // is for the user to specify (e.g.) column<int, primary_key> for
    // the relevant column, but not be required to use column<...> for
    // all others (e.g.):
    //
    //   Ntuple<column<int, primary_key>, double, string>;

    template <typename T>
    struct permissive_column : Column<T> {
      using Column<T>::Column;
      using element_type = T;
    };

    template <typename T>
    struct permissive_column<Column<T>> : permissive_column<T> {
      using permissive_column<T>::permissive_column;
    };

  } // Namespace detail.

} // Namespace hep_hpc.

#endif /* hep_hpc_detail_column_hpp */

// Local Variables:
// mode: c++
// End:
