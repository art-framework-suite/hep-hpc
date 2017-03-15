#ifndef hep_hpc_Ntuple_hpp
#define hep_hpc_Ntuple_hpp

// =====================================================================
//
// Ntuple
//
// The Ntuple class template is an interface for inserting into an
// SQLite database in a type-safe and thread-safe manner.  It is not
// intended for this facility to provide facilities for making SQLite
// database queries.  For querying, consider using the
// 'cet::sqlite::select' utilities.
//
// WARNING: At the moment, two Ntuple instances that access the same
//   database will NOT be thread-safe because locking is disabled to
//   accommodate NFS.  This will be adjusted in a future commit so
//   that access to a database is appropriately serialized across
//   Ntuple instances.
//
// Construction
// ------------
//
// There are two Ntuple c'tor signatures with opposite semantics:
//
//   (1) The c'tor that accepts a database handle object of type
//        (convertible to) sqlite3* implies a non-owning Ntuple object
//        that does not close the SQLite database whenever the Ntuple
//        d'tor is called.
//
//   (2) The c'tor that accepts a filename implies an owning object,
//       for which sqlite3_close IS called whenever the Ntuple object
//       is destroyed.
//
// In addition to the SQLite table name that the constructed Ntuple
// object will refer, a set of column names is also required in the
// form of an std::array object.
//
// See the Ntuple definition below for the full c'tor signatures.
//
// Template arguments
// ------------------
//
// The template arguments supplied for the Ntuple type indicate the
// type of the column.  The following Ntuple specifications are
// identical in semantics:
//
//   Ntuple<int, double, string> n1 {...};
//   Ntuple<column<int>, column<double>, column<string>> n2 {...};
//
// both of which indicate three columns with SQLite types of INTEGER,
// NUMERIC, and TEXT, respectively.  Specifying the 'column' template
// is necessary ONLY when a column constraint is desired (e.g.):
//
//   Ntuple<column<int, primary_key>, double, string> n3 {...};
//
// The names of the columns are provided as a c'tor argument (see
// below).
//
// Intended use
// ------------
//
// There are two public modifying functions that can be called:
//
//    Ntuple::insert(...)
//    Ntuple::flush()
//
// Calling 'insert' will add items to the internal buffer until the
// maximum buffer size has been reached, at which point, the contents
// of the buffer will be automatically flushed to the SQLite database.
//
// Calling 'flush' is necessary only when no more items will be
// inserted into the Ntuple AND querying of the Ntuple is required
// immediately thereafter.  The Ntuple d'tor automatically calls flush
// whenever the Ntuple object goes out of scope.
//
// Examples of use
// ---------------
//
//    using namespace cet::sqlite;
//
//    // Owning use case
//    Ntuple<string, int, int, int> bdays {"bdays.db", "birthdays", {"Name", "Day", "Month", "Year"}};
//    bdays.insert("Betty", 9, 24, 1947);
//    bdays.insert("David", 3, 12, 2015);
//
//    // Non-owning use case
//    Connection c {"languages.db"};
//    Ntuple<string, string> langs {c, "europe", {"Country","Language"}};
//    langs.insert("Germany", "German");
//    langs.insert("Switzerland", "German");
//    langs.insert("Switzerland", "Italian");
//    langs.insert("Switzerland", "French");
//    langs.insert("Switzerland", "Romansh");
//    langs.flush();
//
//    query_result<string> ch;
//    ch << select("Languange").from(c, "europe").where("Country='Switzerland'");
//    // see cet::sqlite::select documentation regarding using query_result.
//
// -----------------------------------------------------------
//
// Technical notes:
//
//   In principle, one could use a concurrent container to prevent
//   having to lock whenever an insert is done.  However, since a
//   flush occurs whenever the buffer max is reached, the buffer must
//   be protected from any modification until the flush is complete.
//   A lock is therefore inevitable.  We could probably optimize by
//   using an atomic variable to protect against modification only
//   when a flush is being done.  This is a potential optimization to
//   keep in mind.

// ===========================================================

#include "hep_hpc/H5File.hpp"
#include "hep_hpc/detail/column.hpp"
// #include "hep_hpc/detail/createDataStructure.hpp"

#include "hdf5.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace hep_hpc {
  template <typename... Args>
  class Ntuple {
public:
    template <typename T> using column = detail::column<T>;
    template <typename T> using permissive_column = detail::permissive_column<T>;

    // Elements of row are unique_ptr's so that it is possible to bind
    // to a null parameter.
    template <typename T>
    using element_t = std::unique_ptr<typename permissive_column<T>::element_type>;

    using row_t = std::tuple<element_t<Args>...>;
    static constexpr auto nColumns = std::tuple_size<row_t>::value;
    using name_array = detail::name_array<nColumns>;

    // Non-owning version--does not close the file when the d'tor is
    // called.
    Ntuple(H5File && file,
           std::string const& name,
           name_array const& columns,
           bool overwriteContents = false,
           std::size_t bufsize = 1000ull);

    // Owning version--sqlite3_close is called on the internally owned
    // database connection handle.
    Ntuple(std::string const& filename,
           std::string const& tablename,
           name_array const& columns,
           bool overwriteContents = false,
           std::size_t bufsiz = 1000ull);

    ~Ntuple() noexcept;

    std::string const& name() const { return name_; }

    void insert(Args const...);
    void flush();

    // Disable copying
    Ntuple(Ntuple const&) = delete;
    Ntuple& operator=(Ntuple const&) = delete;

private:

    static constexpr auto iSequence = std::make_index_sequence<nColumns>();

    // This is the c'tor that does all of the work.  It exists so that
    // the Args... and column-names array can be expanded in parallel.
    template <std::size_t... I>
    Ntuple(H5File && file,
           std::string const& name,
           name_array const& columns,
           bool overwriteContents,
           std::size_t bufsize,
           bool ownsFile,
           std::index_sequence<I...>);

    int flush_no_throw();

    H5File file_;
    std::string name_;
    std::size_t max_;
    bool ownsFile_;
    std::vector<row_t> buffer_ {};
    std::recursive_mutex mutex_ {};
  };

} // Namespace hep_hpc.

template <typename... Args>
template <std::size_t... I>
hep_hpc::Ntuple<Args...>::Ntuple(H5File && file,
                                  std::string const& name,
                                  name_array const& cnames [[gnu::unused]],
                                  bool const overwriteContents [[gnu::unused]],
                                  std::size_t const bufsize,
                                  bool const ownsFile,
                                  std::index_sequence<I...>) :
  file_{std::move(file)},
  name_{name},
  max_{bufsize},
  ownsFile_{ownsFile}
{
  if (!file_) {
    throw std::runtime_error("Attempt to create Ntuple with invalid H5File.");
  }

  // Create data structure.
  // detail::createHDF5DataStructure(file, overwriteContents, name, column<Args>{cnames[I]}...);

  // Reserve buffer space.
  buffer_.reserve(bufsize);
}

template <typename... Args>
hep_hpc::Ntuple<Args...>::Ntuple(H5File && file,
                                  std::string const& name,
                                  name_array const& cnames,
                                  bool const overwriteContents,
                                  std::size_t const bufsize) :
  Ntuple{std::move(file), name, cnames, overwriteContents, bufsize, false, iSequence}
{}

template <typename... Args>
hep_hpc::Ntuple<Args...>::Ntuple(std::string const& filename,
                                  std::string const& name,
                                  name_array const& cnames,
                                  bool const overwriteContents,
                                  std::size_t const bufsize) :
  Ntuple{H5File(filename), name, cnames, overwriteContents, bufsize, true, iSequence}
{}

template <typename... Args>
hep_hpc::Ntuple<Args...>::~Ntuple() noexcept
{
  if (flush_no_throw() != 0) {
    std::cerr << "HDF5 failure while flushing.\n";
  }
}

template <typename... Args>
void
hep_hpc::Ntuple<Args...>::insert(Args const... args)
{
  std::lock_guard<decltype(mutex_)> lock {mutex_};
  if (buffer_.size() == max_) {
    flush();
  }
  buffer_.emplace_back(std::make_unique<Args>(args)...);
}

template <typename... Args>
int
hep_hpc::Ntuple<Args...>::flush_no_throw()
{
  std::lock_guard<decltype(mutex_)> lock {mutex_};
  for (auto const& r [[gnu::unused]]: buffer_) {
    auto rc = 0; // Transfer data to file.
    if (rc != 0) {
      return rc;
    }
  }
  buffer_.clear();
  return 0;
}

template <typename... Args>
void
hep_hpc::Ntuple<Args...>::flush()
{
  // No lock here -- lock held by flush_no_throw;
  if (flush_no_throw() != 0) {
    throw std::runtime_error("HDF5 write failure.");
  }
}

#endif /* hep_hpc_Ntuple_hpp */

// Local Variables:
// mode: c++
// End:
