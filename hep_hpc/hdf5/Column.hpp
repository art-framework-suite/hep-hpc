#ifndef hep_hpc_hdf5_Column_hpp
#define hep_hpc_hdf5_Column_hpp
////////////////////////////////////////////////////////////////////////
// template <typename T, size_t NDIMS = 1> hep_hpc::hdf5::Column;
//
//   A template representing an Ntuple column whose elements are an
//   NDIMS-dimensional array of T. The cardinality of each dimension is
//   specified at construction time; the rank is specified at compile
//   time.
//
//   Every supported T requires a specialization.
//
////////////////////////////////////
// template <size_t SZ>
// using hep_hpc::hdf5::fstring_t = std::array<char, SZ>;
//
//   for straightforward representation of fixed-length strings.
//
////////////////////////////////////
// enum class hep_hpc::hdf5::TranslationMode;
//
//   Force representation on disk regardless of current architecture.
//   Values are:
//
//   * NONE
//
//   * IEEE_STD_LE
//
//   * IEEE_STD_BE
//
////////////////////////////////////
// hep_hpc::hdf5::Column details.
//
//////////////////
// Types
//
// using dims_array = std::array<size_t, NDIMS>;
//
//////////////////
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
//////////////////
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
// static hid_t engine_type(TranslationMode mode =
//                          TranslationMode::NONE);
//
//   The HDF5 handle to the basic HDF5 type by which T is
//   represented. If mode is specifed and not TranslationMode::NONE, the
//   native representation will be translated to the specifed format for
//   file storage.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/hdf5/Datatype.hpp"
#include "hep_hpc/hdf5/Exception.hpp"

#include "hdf5.h"

#include <array>
#include <cstdint>
#include <numeric>
#include <string>

namespace hep_hpc {
  namespace hdf5 {
    template<typename T, size_t NDIMS = 1>
    struct Column;

    template <size_t SZ>
    using fstring_t = std::array<char, SZ>;

    namespace detail {
      enum class TranslationMode : uint8_t {
        NONE,
          IEEE_STD_LE, // Little-endian, IEEE-754 floating point.
          IEEE_STD_BE // Big-endian, IEEE-754 floating point.
          };
    }

    using detail::TranslationMode;
  }
}

////////////////////////////////////////////////////////////////////////
// No user-serviceable parts.

#define ENGINE_TYPE(NATIVE, IEEE_LE, IEEE_BE)   \
  using namespace std::string_literals;         \
  using std::to_string;                         \
  hid_t result;                                 \
  switch (mode) {                               \
  case TranslationMode::NONE:                   \
    result = NATIVE;                            \
    break;                                      \
  case TranslationMode::IEEE_STD_LE:            \
    result = IEEE_LE;                           \
    break;                                      \
  case TranslationMode::IEEE_STD_BE:            \
    result = IEEE_BE;                           \
    break;                                      \
  default:                                      \
    throw Exception("Un-handled translation mode: "s + to_string((int)mode)); \
  } \
    return result;

namespace hep_hpc {
  namespace hdf5 {
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

        column_base(std::string n, dims_array dims)
          :
          name_{std::move(n)},
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
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE(H5T_NATIVE_DOUBLE, H5T_IEEE_F64LE, H5T_IEEE_F64BE) }
    };

    template <size_t NDIMS>
    struct Column<float, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE(H5T_NATIVE_FLOAT, H5T_IEEE_F32LE, H5T_IEEE_F32BE) }
    };

    template <size_t NDIMS>
    struct Column<int8_t, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE(H5T_NATIVE_SCHAR, H5T_STD_I8LE, H5T_STD_I8BE) }
    };

    template <size_t NDIMS>
    struct Column<short, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE(H5T_NATIVE_SHORT, H5T_STD_I16LE, H5T_STD_I16BE) }
    };

    template <size_t NDIMS>
    struct Column<int, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE(H5T_NATIVE_INT, H5T_STD_I32LE, H5T_STD_I32BE) }
    };

    template <size_t NDIMS>
    struct Column<long, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
#ifdef __APPLE__
        { ENGINE_TYPE(H5T_NATIVE_LONG, H5T_STD_I32LE, H5T_STD_I32BE) }
#else
        { ENGINE_TYPE(H5T_NATIVE_LONG, H5T_STD_I64LE, H5T_STD_I64BE) }
#endif
    };

    template <size_t NDIMS>
    struct Column<long long, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE(H5T_NATIVE_LLONG, H5T_STD_I64LE, H5T_STD_I64BE) }
    };

    template <size_t NDIMS>
    struct Column<uint8_t, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE(H5T_NATIVE_UCHAR, H5T_STD_U8LE, H5T_STD_U8BE) }
    };

    template <size_t NDIMS>
    struct Column<unsigned short, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE(H5T_NATIVE_USHORT, H5T_STD_U16LE, H5T_STD_U16BE) }
    };

    template <size_t NDIMS>
    struct Column<unsigned int, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE( H5T_NATIVE_UINT, H5T_STD_U32LE, H5T_STD_U32BE) }
    };

    template <size_t NDIMS>
    struct Column<unsigned long, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
#ifdef __APPLE__
        { ENGINE_TYPE(H5T_NATIVE_ULONG, H5T_STD_U32LE, H5T_STD_U32BE) }
#else
        { ENGINE_TYPE(H5T_NATIVE_ULONG, H5T_STD_U64LE, H5T_STD_U64BE) }
#endif
    };

    template <size_t NDIMS>
    struct Column<unsigned long long, NDIMS> : detail::column_base<NDIMS> {
      using detail::column_base<NDIMS>::column_base;
      static hid_t engine_type(TranslationMode mode = TranslationMode::NONE)
        { ENGINE_TYPE(H5T_NATIVE_ULLONG, H5T_STD_U64LE, H5T_STD_U64BE) }
    };

    template <size_t NDIMS>
    struct Column<char const *, NDIMS> : detail::column_base<NDIMS> {

      Column(std::string n, detail::dims_array<NDIMS> dims)
        :
        detail::column_base<NDIMS>(std::move(n), std::move(dims)),
        STRING_TYPE_(H5Tcopy(H5T_C_S1))
        { H5Tset_size(STRING_TYPE_, H5T_VARIABLE); }

      hid_t engine_type(TranslationMode) const
        { return STRING_TYPE_; }

  private:
      hdf5::Datatype STRING_TYPE_;
    };

    template <>
    struct Column<char const *, 1ull> : detail::column_base<1ull> {
      Column(std::string n, size_t dim)
        :
        detail::column_base<1ull>(std::move(n), dim),
        STRING_TYPE_(H5Tcopy(H5T_C_S1))
        { H5Tset_size(STRING_TYPE_, H5T_VARIABLE); }

      hid_t engine_type(TranslationMode) const
        { return STRING_TYPE_; }

  private:
      hdf5::Datatype STRING_TYPE_;
    };

    template <size_t NDIMS>
    struct Column<char *, NDIMS> : detail::column_base<NDIMS> {

      Column(std::string n, detail::dims_array<NDIMS> dims)
        :
        detail::column_base<NDIMS>(std::move(n), std::move(dims)),
        STRING_TYPE_(H5Tcopy(H5T_C_S1))
        { H5Tset_size(STRING_TYPE_, H5T_VARIABLE); }

      hid_t engine_type(TranslationMode) const
        { return STRING_TYPE_; }

  private:
      hdf5::Datatype STRING_TYPE_;
    };

    template <>
    struct Column<char *, 1ull> : detail::column_base<1ull> {
      Column(std::string n, size_t dim)
        :
        detail::column_base<1ull>(std::move(n), dim),
        STRING_TYPE_(H5Tcopy(H5T_C_S1))
        { H5Tset_size(STRING_TYPE_, H5T_VARIABLE); }

      hid_t engine_type(TranslationMode) const
        { return STRING_TYPE_; }

  private:
      hdf5::Datatype STRING_TYPE_;
    };

    template <size_t NDIMS>
    struct Column<std::string, NDIMS> : detail::column_base<NDIMS> {

      Column(std::string n, detail::dims_array<NDIMS> dims)
        :
        detail::column_base<NDIMS>(std::move(n), std::move(dims)),
        STRING_TYPE_(H5Tcopy(H5T_C_S1))
        { H5Tset_size(STRING_TYPE_, H5T_VARIABLE); }

      hid_t engine_type(TranslationMode) const
        { return STRING_TYPE_; }

  private:
      hdf5::Datatype STRING_TYPE_;
    };

    template <>
    struct Column<std::string, 1ull> : detail::column_base<1ull> {
      Column(std::string n, size_t dim)
        :
        detail::column_base<1ull>(std::move(n), dim),
        STRING_TYPE_(H5Tcopy(H5T_C_S1))
        { H5Tset_size(STRING_TYPE_, H5T_VARIABLE); }

      hid_t engine_type(TranslationMode) const
        { return STRING_TYPE_; }

  private:
      hdf5::Datatype STRING_TYPE_;
    };

    template <size_t SZ, size_t NDIMS>
    struct Column<fstring_t<SZ>, NDIMS> : detail::column_base<NDIMS> {

      Column(std::string n, detail::dims_array<NDIMS> dims)
        :
        detail::column_base<NDIMS>(std::move(n), std::move(dims)),
        STRING_TYPE_(H5Tcopy(H5T_C_S1))
        { H5Tset_size(STRING_TYPE_, SZ); }

      hid_t engine_type(TranslationMode) const
        { return STRING_TYPE_; }

  private:
      hdf5::Datatype STRING_TYPE_;
    };

    template <size_t SZ>
    struct Column<std::array<char, SZ>, 1ull> : detail::column_base<1ull> {
      Column(std::string n, size_t dim = 1ull)
        :
        detail::column_base<1ull>(std::move(n), dim),
        STRING_TYPE_(H5Tcopy(H5T_C_S1))
        { H5Tset_size(STRING_TYPE_, SZ); }

      hid_t engine_type(TranslationMode) const
        { return STRING_TYPE_; }

  private:
      hdf5::Datatype STRING_TYPE_;
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

  } // Namespace hdf5.

} // Namespace hep_hpc.

#endif /* hep_hpc_hdf5_Column_hpp */

// Local Variables:
// mode: c++
// End:
