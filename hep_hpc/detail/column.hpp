#ifndef hep_hpc_detail_column_hpp
#define hep_hpc_detail_column_hpp

// ===================================================================
// column<T, Constraints...>
//
// SQLite tables are composed of columns that are specified in SQLite
// as (e.g):
//
//  CREATE TABLE workers(FirstName TEXT, LastName TEXT);
//
// where the fields in parentheses denote two columns with the names
// 'FirstName' and 'LastName', both of which are of the type 'TEXT'.
//
// The column template allows a user to specify a column using native
// C++ types.  For example, the above SQLite statement can be achieved
// and executed in C++ using the create_table command:
//
//   cet::sqlite::create_table(db,
//                             "workers",
//                             column<string>{"FirstName"},
//                             column<string>{"LastName"});
//
// Column constraints are also allowed (e.g.):
//
//   column<int, primary_key, autoincrement>{"id"}
//
// See the notes in cetlib/sqlite/detail/column_constraint.h.
// ===================================================================

#include "hdf5.h"

#include <array>
#include <string>
#include <utility>
#include <tuple>

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

  // column<T> is a containing struct that knows its C++ type (T)
  // and the sqlite translation (sqlite_type()).  There is no
  // implementation for the general case; the template must be
  // specialized for each supported type.
  template <typename T>
  struct column;

  template <>
  struct column<double> : column_base {
    using column_base::column_base;
    using type = double;
    static hid_t engine_type() { return H5T_NATIVE_DOUBLE; }
  };

  template <>
  struct column<float> : column_base {
    using column_base::column_base;
    using type = float;
    static hid_t engine_type() { return H5T_NATIVE_FLOAT; }
  };

  template <>
  struct column<int> : column_base {
    using column_base::column_base;
    using type = int;
    static hid_t engine_type() { return H5T_NATIVE_INT; }
  };

  template <>
  struct column<long> : column_base {
    using column_base::column_base;
    using type = long;
    static hid_t engine_type() { return H5T_NATIVE_LONG; }
  };

  template <>
  struct column<long long> : column_base {
    using column_base::column_base;
    using type = long long;
    static hid_t engine_type() { return H5T_NATIVE_LLONG; }
  };

  template <>
  struct column<unsigned int> : column_base {
    using column_base::column_base;
    using type = unsigned int;
    static hid_t engine_type() { return H5T_NATIVE_UINT; }
  };

  template <>
  struct column<unsigned long> : column_base {
    using column_base::column_base;
    using type = unsigned long;
    static hid_t engine_type() { return H5T_NATIVE_ULONG; }
  };

  template <>
  struct column<unsigned long long> : column_base {
    using column_base::column_base;
    using type = unsigned long long;
    static hid_t engine_type() { return H5T_NATIVE_ULLONG; }
  };

  // template <int LEN>
  // struct column<char<LEN>> : column_base {
  //   using column_base::column_base;
  //   using type = std::string;
  //   static constexpr hid_t engine_type() { return H5T_NATIVE_CHAR; }
  // };

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
  struct permissive_column : column<T> {
    using column<T>::column;
    using element_type = T;
  };

  template <typename T>
  struct permissive_column<column<T>> : permissive_column<T> {
    using permissive_column<T>::permissive_column;
  };

  } // Namespace detail.

} // Namespace hep_hpc.

#endif /* hep_hpc_detail_column_hpp */

// Local Variables:
// mode: c++
// End:
