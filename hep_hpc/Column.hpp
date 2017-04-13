#ifndef hep_hpc_Column_hpp
#define hep_hpc_Column_hpp
////////////////////////////////////////////////////////////////////////
// template <typename T, size_t NDIMS = 1> hep_hpc::Column
//
// A template representing an Ntuple column whose elements are an
// NDIMS-dimensional array of T. The cardinality of each dimension is
// specified at construction time; the rank is specified at compile
// time.
//
// Every supported T requires a specialization.
//
////////////////////////////////////
//
// Types
//
// using dims_array = std::array<size_t, NDIMS>;
//
// Constructors:
//
// Column<T, 1>::Column(std::string const & colname,
//                      size_t dim_size = 1ull);
//
//   e.g. Column<int>("MyIntArray", 12) would describe a column
//   whose elements were 1-dimensional arrays of length 12.
//
// Column<T, N>::Column(std::string const & colname,
//                      dims_array dims);
//
//   e.g. Column<int, 2>("MyIntMatrix", {3, 3}) woud describe a column
//   whose elements were 3x3 matrices.
//
// Members
//
// auto const name() const;
//
//   The column name.
//
// static constexpr size_t nDims();
//
//   The rank of the column elements.
//
// size_t const * dims() const;
//
//   A pointer to a contiguous sequence whose members are the
//   cardinality of each dimension.
//
// size_t elementSize() const;
//
//   The number of basic elements of type T in a single element of the
//   column.
//
// static hid_t engine_type();
//
//   The HDF5 handle to the basic HDF5 type by which T is represented.
//
////////////////////////////////////////////////////////////////////////
#include "hdf5.h"

#include <array>
#include <numeric>
#include <string>

namespace hep_hpc {
  template<typename T, size_t NDIMS = 1>
  struct Column;
}

////////////////////////////////////////////////////////////////////////
// No user-serviceable parts.
namespace hep_hpc {

  namespace detail {
    template <size_t NDIMS>
    using dims_array = std::array<size_t, NDIMS>;

    template <size_t NDIMS>
    class column_base {
  public:
      static_assert(NDIMS < H5S_MAX_RANK,
                    "Cannot represent data in HDF5 as a column of elements of "
                    "more than (H5S_MAX_RANK-1) dimensions");

      using dims_array = detail::dims_array<NDIMS>;

      column_base(std::string const& n, dims_array dims)
        :
        name_{n},
        dims_{dims},
        elementSize_{std::accumulate(std::cbegin(dims_),
                                     std::cend(dims_),
                                     1ull,
                                     std::multiplies<size_t>())}
        { }
      auto const& name() const { return name_; }
      static constexpr size_t nDims() { return NDIMS; }
      size_t const * dims() const { return dims_.data(); }
      size_t elementSize() const { return elementSize_; }

  private:
      std::string const name_;
      std::array<size_t, NDIMS> const dims_;
      size_t const elementSize_;
    };

    template <>
    class column_base<1ull> {
  public:
      column_base(std::string const& n, size_t dim = 1ull) : name_{n}, dim_{dim}
        { }
      auto const & name() const { return name_; }
      static constexpr size_t nDims() { return 1ull; }
      size_t const * dims() const { return &dim_; }
      size_t elementSize() const { return dim_; }
  private:
      std::string name_;
      size_t dim_;
    };
  }

  template <size_t NDIMS>
  struct Column<double, NDIMS> : detail::column_base<NDIMS> {
    using detail::column_base<NDIMS>::column_base;
    static hid_t engine_type() { return H5T_NATIVE_DOUBLE; }
  };

  template <size_t NDIMS>
  struct Column<float, NDIMS> : detail::column_base<NDIMS> {
    using detail::column_base<NDIMS>::column_base;
    static hid_t engine_type() { return H5T_NATIVE_FLOAT; }
  };

  template <size_t NDIMS>
  struct Column<int, NDIMS> : detail::column_base<NDIMS> {
    using detail::column_base<NDIMS>::column_base;
    static hid_t engine_type() { return H5T_NATIVE_INT; }
  };

  template <size_t NDIMS>
  struct Column<long, NDIMS> : detail::column_base<NDIMS> {
    using detail::column_base<NDIMS>::column_base;
    static hid_t engine_type() { return H5T_NATIVE_LONG; }
  };

  template <size_t NDIMS>
  struct Column<long long, NDIMS> : detail::column_base<NDIMS> {
    using detail::column_base<NDIMS>::column_base;
    static hid_t engine_type() { return H5T_NATIVE_LLONG; }
  };

  template <size_t NDIMS>
  struct Column<unsigned int, NDIMS> : detail::column_base<NDIMS> {
    using detail::column_base<NDIMS>::column_base;
    static hid_t engine_type() { return H5T_NATIVE_UINT; }
  };

  template <size_t NDIMS>
  struct Column<unsigned long, NDIMS> : detail::column_base<NDIMS> {
    using detail::column_base<NDIMS>::column_base;
    static hid_t engine_type() { return H5T_NATIVE_ULONG; }
  };

  template <size_t NDIMS>
  struct Column<unsigned long long, NDIMS> : detail::column_base<NDIMS> {
    using detail::column_base<NDIMS>::column_base;
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

    template <typename T, size_t NDIMS = 1ull>
    struct permissive_column : Column<T, NDIMS> {
      using Column<T, NDIMS>::Column;
      using element_type = T;
    };

    template <typename T>
    struct permissive_column<Column<T> > : permissive_column<T> {
      using permissive_column<T>::permissive_column;
    };

    template <typename T, size_t NDIMS>
    struct permissive_column<Column<T, NDIMS> > : permissive_column<T, NDIMS> {
      using permissive_column<T, NDIMS>::permissive_column;
    };

  } // Namespace detail.

} // Namespace hep_hpc.

#endif /* hep_hpc_Column_hpp */

// Local Variables:
// mode: c++
// End:
