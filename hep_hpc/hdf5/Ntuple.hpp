#ifndef hep_hpc_hdf5_Ntuple_hpp
#define hep_hpc_hdf5_Ntuple_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::Ntuple
//
// An implementation of an Ntuple with an HDF5 backend.
//
////////////////////////////////////
// Overview.
//
// * This is a variadic template (takes an arbitrary number of template
//   arguments).
//
// * Each template argument represents a column of the Ntuple.
//
// * Insertions are row-wise, storage is column-wise.
//
////////////////////////////////////
// Defined types
//
// column_info_t
//
//   This type may be considered to be defined as an
//   std::tuple<<column-type>...>, where <column-type> is determined by
//   the template arguments to Ntuple.
//
//   column_info_t is used to describe all of the column information for
//   the Ntuple. It enables the use of a brace-enclosed initializer to
//   describe the column information in the constructor of Ntuple (see
//   below).
//
////////////////////////////////////
// Interface
//
// Template arguments should be specified as the data element type
// (double, int, etc.) or of type hep_hpc::Column (see
// hep_hpc/Column.hpp for details). Supported currently are columns of
// n-dimensional fixed-size arrays of:
//
// * basic arithmetic types ({u,}int8_t,{un,}signed {short, int, long,
//   long long}, float, double, long double). char is explicitly
//   disallowed: see string storage below;
//
// * hdstudy::hdf5::fstring_t<N> a.k.a. std::array<char, N>
//   (fixed-length string support);
//
// * std::string, char const * or char * (variable-length string
//   support). However, see the notes for insert() below.
//
// N.B. We recommend using hep_hpc::hdf5::make_ntuple (see
// hep_hpc/hdf5/make_ntuple.hpp) with hep_hpc::hdf5::make_column() (see
// hep_hpc/hdf5/make_column.hpp) and/or
// hep_hpc::hdf5::make_scalar_column() for all but the very simplest of
// ntuples. Reviewing the interface below, however, remains useful for
// understanding the features of the Ntuple class so created and how it
// should be used.
//
////////////////////////////////////
// Constructors
//
// Ntuple<Args...>(hid_t file,
//                 std::string tablename,
//                 column_info_t columns,
//                 [TranslationMode mode,]
//                 OverwriteFlag overwriteContents = <default>,
//                 std::size_t bufsize = <default>);
//
// Ntuple<Args...>(std::string filename,
//                 std::string tablename,
//                 column_info_t columns,
//                 [TranslationMode mode,]
//                 std::size_t bufsize = <default>);
//
//   Create an Ntuple tied to an HDF5 file with column types specified
//   by Args and column information specified by columns. A valid
//   columns parameter would be a brace-enclosed initializer list whose
//   elements are either:
//
//     1) a string representing the column name if the column's element
//        type is a scalar (corresponding Arg should be <basic-type> or
//        Column<<basic-type>, 1>);
//
//     2) a brace-enclosed initializer list: {<string>, n} if the
//        column's element type is a one-dimensional array of length n
//        (corresponding Arg should be <basic-type> or
//        Column<<basic-type>, 1>); or
//
//     3) a brace-enclosed initializer list {<string>, {n, ...}} if the
//        column's element type is of rank 2 or greater (corresponding
//        Arg should be Column<<basic-type>, rank>).
//
//   If hid_t is provided, caller is responsible for file resource
//   management. If filename is provided and file exists, it is
//   truncated.
//
//   If TranslationMode is specified (see hep_hpc/Column.hpp for
//   details), then the representation on disk is specified (e.g. as
//   IEEE 754 and little-endian). Not specifying is equivalent to
//   specifying TranslationMode::NONE. Use for the rare case when data
//   are to be produced on a machine with a different architecture than
//   that of the machines where the data are to be used.
//
//   overwriteContents controls whether an existing entity of name
//   <tablename> would be overwritten or not (if not, an exception is
//   thrown currently). Its value, if specified, should be
//   hep_hpc::hdf5::OverwriteFlag::NO or
//   hep_hpc::hdf5::OverwriteFlag::YES.
//
//   Buffer size controls how many rows are cached in memory before
//   being flushed to the file; defaults to 1000 if not specified.
//
//   Insertion is row-wise; storage is column-wise.
//
////////////////////////////////////
// ~Ntuple() noexcept
//
//   This nontrivial destructor will ensure that all existing buffered
//   data have been flushed to the HDF5, and the file and all associated
//   HDF5 entities have been closed
//
////////////////////////////////////
// std::string name() const;
//
//   Return the name of the group containing the Ntuple data.
//
////////////////////////////////////
// Group const & group() const;
//
//   Give access to the HDF5 group encapsulating the datasets for this
//   Ntuple.
//
////////////////////////////////////
// static constexpr std::size_t nColumns();
//
//    Static function returning the number of columns defined for the
//    ntuple.
//
////////////////////////////////////
// std::array<Dataset, ncolumns()> const & datasets() const;
//
//   Give access to the HDF5 datasets representing the data in the file.
//
////////////////////////////////////
// template <typename T>
// void insert(T...);
//
//   Insert a row of data. Each argument is expected to be a pointer to
//   the basic element type T of each column or (if scalar) a T by value
//   (it is a compile-time error to fail to provide one argument per
//   column). If the argument is not nullptr, it is expected to be a
//   pointer to a contiguous sequence of items of the column's basic
//   type (e.g. double) of length Column::elementSize(). If the argument
//   is nullptr, then the buffer will be filled with
//   Column::elementSize() default-constructed items of type T. This
//   contiguous sequence must be organized according to the HDF5
//   description for n-dimensional array representation: right-most
//   index moves fastest.
//
//   N.B. Variable-length strings are supported with a basic element
//   type of std::string or const char * (or char *). In the case of
//   std::string, you are providing to insert() a pointer to a
//   contiguous array of std::string, each of which is copied at
//   insert() time. In the case of the char * types, one therefore
//   provides to insert() a char const * * (or char * *) which points to
//   a contiguous array of char const * (or char *), /each of which must
//   continue to point to a valid null-terminated character string until
//   the buffer has been flushed/.
//
//   If the buffer is full, it will be flushed prior to the data being
//   inserted.
//
////////////////////////////////////
//
// void flush()
//
//   Flush the currently-buffered data to file.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/Utilities/detail/compiler_macros.hpp"
#include "hep_hpc/Utilities/detail/index_sequence.hpp"
#include "hep_hpc/hdf5/Column.hpp"
#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/detail/NtupleDataStructure.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "hdf5.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace hep_hpc {
  namespace hdf5 {
    template <typename... Args>
    class Ntuple;

    enum class NtupleOverwriteFlag : bool { NO, YES };

  } // Namespace hdf5.
} // Namespace hep_hpc.

template <typename... Args>
class hep_hpc::hdf5::Ntuple {
public:
  using column_info_t = std::tuple<detail::permissive_column<Args>...>;

  // 1.
  Ntuple(hid_t file,
         std::string tablename,
         column_info_t columns,
         NtupleOverwriteFlag overwriteContents = NtupleOverwriteFlag::NO,
         std::size_t bufsize = 1000ull);

  // 2.
  Ntuple(hid_t file,
         std::string tablename,
         column_info_t columns,
         TranslationMode mode,
         NtupleOverwriteFlag overwriteContents = NtupleOverwriteFlag::NO,
         std::size_t bufsize = 1000ull);

  // 3.
  Ntuple(std::string filename,
         std::string tablename,
         column_info_t columns,
         std::size_t bufsiz = 1000ull);

  // 4.
  Ntuple(std::string filename,
         std::string tablename,
         column_info_t columns,
         TranslationMode mode,
         std::size_t bufsiz = 1000ull);

  ~Ntuple() noexcept;

  File const & file() const;
  std::string const & name() const;
  Group const &  group() const;
  static constexpr std::size_t nColumns() { return sizeof...(Args); }
  std::array<Dataset, nColumns()> const & datasets() const;

  template <typename... T>
  void insert(T && ...);
  void flush();

  // Enable moving
  Ntuple(Ntuple &&) = default;
  Ntuple & operator=(Ntuple &&) = default;

  // Disable copying
  Ntuple(Ntuple const&) = delete;
  Ntuple & operator=(Ntuple const&) = delete;

private:
  static_assert(nColumns() > 0, "Ntuple with zero types is meaningless");

  template <typename T>
  using Element_t = typename detail::permissive_column<T>::element_type;

  static constexpr hep_hpc::detail::make_index_sequence<nColumns()> iSequence()
    { return hep_hpc::detail::make_index_sequence<nColumns()>(); }

  // 5.
  //
  // This is the c'tor that does all of the work. It exists so that the
  // Args... and column-names array can be expanded in parallel.
  template <std::size_t... I>
  Ntuple(File file,
         std::string tablename,
         column_info_t columns,
         TranslationMode mode,
         NtupleOverwriteFlag overwriteContents,
         std::size_t bufsize,
         hep_hpc::detail::index_sequence<I...>);

  template <size_t... I>
  int flush_(hep_hpc::detail::index_sequence<I...>);

  std::tuple<std::vector<Element_t<Args> >...> buffers_;

  File file_;
  std::string name_;
  std::array<size_t, nColumns()> max_;
  std::unique_ptr<std::recursive_mutex> mutex_ {};
  detail::NtupleDataStructure<Args...> dd_;
};

////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////

namespace hep_hpc {
  namespace hdf5 {
    namespace NtupleDetail {
      File verifiedFile(File file);

      template <size_t I, typename TUPLE, typename COLS, typename... Tail>
      void
      insert(TUPLE & buffers, COLS const & cols,
             typename std::tuple_element<I, COLS>::type::element_type head,
             Tail && ... tail);

      template <size_t I, typename TUPLE, typename COLS, typename... Tail>
      void
      insert(TUPLE & buffers, COLS const & cols,
             typename std::tuple_element<I, COLS>::type::element_type const * head,
             Tail && ... tail);

      template <size_t I, typename TUPLE, typename COLS>
      void
      insert(TUPLE &, COLS const &) { }

      template <typename BUFFER, typename COL>
      herr_t flush_one(BUFFER & buf, Dataset & dset, COL const & col);

      // Special case: shim for std::string.
      template <typename COL>
      herr_t flush_one(std::vector<std::string> & buf,
                       Dataset & dset,
                       COL const & col);

      // Special case: shim for std::array.
      template <int SZ, typename COL>
      herr_t flush_one(std::vector<std::array<char, SZ> > & buf,
                       Dataset & dset,
                       COL const & col);
    } // Namespace NtupleDetail.
  } // Namespace hdf5.
} // Namespace hep_hpc.

// 1.
template <typename... Args>
hep_hpc::hdf5::Ntuple<Args...>::Ntuple(hid_t file,
                                       std::string name,
                                       column_info_t columns,
                                       NtupleOverwriteFlag overwriteContents,
                                       std::size_t const bufsize) :
  Ntuple{File(file),
    std::move(name),
    std::move(columns),
    TranslationMode::NONE,
    overwriteContents,
    bufsize,
    iSequence()}
{}

// 2.
template <typename... Args>
hep_hpc::hdf5::Ntuple<Args...>::Ntuple(hid_t file,
                                       std::string name,
                                       column_info_t columns,
                                       TranslationMode mode,
                                       NtupleOverwriteFlag overwriteContents,
                                       std::size_t const bufsize) :
  Ntuple{File(file),
    std::move(name),
    std::move(columns),
    mode,
    overwriteContents,
    bufsize,
    iSequence()}
{}

namespace {
  [[gnu::unused]]
  hep_hpc::hdf5::PropertyList fileAccessProperties()
  {
    // Ensure we are using the latest available HDF5 file format to write our data.
    hep_hpc::hdf5::PropertyList plist(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(plist, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    return plist;
  }
}

// 3.
template <typename... Args>
hep_hpc::hdf5::Ntuple<Args...>::Ntuple(std::string filename,
                                       std::string name,
                                       column_info_t columns,
                                       std::size_t const bufsize) :
  Ntuple{File(std::move(filename), H5F_ACC_TRUNC, {}, fileAccessProperties()),
    std::move(name), std::move(columns), TranslationMode::NONE, NtupleOverwriteFlag::NO, bufsize, iSequence()}
{}

// 4.
template <typename... Args>
hep_hpc::hdf5::Ntuple<Args...>::Ntuple(std::string filename,
                                       std::string name,
                                       column_info_t columns,
                                       TranslationMode mode,
                                       std::size_t const bufsize) :
  Ntuple{File(std::move(filename), H5F_ACC_TRUNC, {}, fileAccessProperties()),
    std::move(name), std::move(columns), mode, NtupleOverwriteFlag::NO, bufsize, iSequence()}
{}

// 5.
template <typename... Args>
template <std::size_t... I>
hep_hpc::hdf5::Ntuple<Args...>::Ntuple(File file,
                                       std::string name,
                                       column_info_t columns,
                                       TranslationMode mode,
                                       NtupleOverwriteFlag overwriteContents,
                                       std::size_t const bufsize,
                                       hep_hpc::detail::index_sequence<I...>) :
  file_{NtupleDetail::verifiedFile(std::move(file))},
  name_{std::move(name)},
  max_{(std::get<I>(columns).elementSize() * bufsize)...},
  mutex_{new std::recursive_mutex},
  dd_(file_, name_, mode, static_cast<bool>(overwriteContents), std::move(std::get<I>(columns))...)
{
  // Reserve buffer space.
  using swallow = int[];
  // Reserve the right amount of space in each buffer.
  (void) swallow {0, (std::get<I>(buffers_).reserve(max_[I]), 0)...};
}

template <typename... Args>
hep_hpc::hdf5::Ntuple<Args...>::~Ntuple() noexcept
{
  ScopedErrorHandler seh(ErrorMode::HDF5_DEFAULT);
  if (flush_(iSequence()) != 0) {
    std::cerr << "HDF5 failure while flushing.\n";
  }
}

template <typename... Args>
inline
hep_hpc::hdf5::File const &
hep_hpc::hdf5::Ntuple<Args...>::file() const
{
  return file_;
}

template <typename... Args>
inline
std::string const &
hep_hpc::hdf5::Ntuple<Args...>::name() const
{
  return name_;
}

template <typename... Args>
inline
hep_hpc::hdf5::Group const &
hep_hpc::hdf5::Ntuple<Args...>::group() const
{
  return dd_.group;
}

template <typename... Args>
inline
auto
hep_hpc::hdf5::Ntuple<Args...>::datasets() const
-> std::array<Dataset, nColumns()> const &
{
  return dd_.dsets;
}

template <typename... Args>
template <typename... T>
void
hep_hpc::hdf5::Ntuple<Args...>::insert(T && ... args)
{
  static_assert(sizeof...(T) == nColumns(),
                "Number of arguments to insert() must match nColumns().");

  using std::get;
  std::lock_guard<decltype(*mutex_)> lock {*mutex_};
  if (get<0>(buffers_).size() >= max_[0]) {
    flush();
  }
  NtupleDetail::insert<0>(buffers_, dd_.columns, std::forward<T>(args)...);
}

template <typename... Args>
template <size_t... I>
int
hep_hpc::hdf5::Ntuple<Args...>::flush_(hep_hpc::detail::index_sequence<I...>)
{
  using std::get;
  std::lock_guard<decltype(*mutex_)> lock {*mutex_};
  auto const results =
    {(herr_t) 0, NtupleDetail::flush_one(get<I>(buffers_),
                                         get<I>(dd_.dsets),
                                         get<I>(dd_.columns))...};
  return std::any_of(std::begin(results),
                     std::end(results),
                     [](herr_t const res) { return res != 0; });
}

template <typename BUFFER, typename COL>
herr_t
hep_hpc::hdf5::NtupleDetail::
flush_one(BUFFER & buf, Dataset & dset, COL const & col)
{
  using std::get;
  herr_t rc = -1;
  // Obtain the current dataspace for this dataset.
  auto dspace = Dataspace{ErrorController::call(&H5Dget_space, dset)};
  std::array<hsize_t, COL::nDims() + 1ull> filedims, filemaxdims, offsets {0}, nElements;
  if (H5Sget_simple_extent_dims(dspace, filedims.data(), filemaxdims.data()) !=
      COL::nDims() + 1ull) {
    return rc;
  }
  nElements[0] = buf.size() / col.elementSize();
  std::copy(col.dims(), col.dims() + col.nDims(), std::begin(nElements) + 1ull);
  offsets[0] = filedims[0];
  // Extend long dimension.
  filedims[0] += nElements[0];
  // Update dataset.
  if ((rc = ErrorController::call(&H5Dset_extent,dset, filedims.data())) != 0) {
    return rc;
  }
  // Need to get fresh dataspace info after updating dataset.
  dspace = Dataspace{ErrorController::call(&H5Dget_space, dset)};
  // Data selection for write.
  if ((rc = ErrorController::call(&H5Sselect_hyperslab,
                                  dspace,
                                  H5S_SELECT_SET,
                                  offsets.data(),
                                  nullptr,
                                  nElements.data(),
                                  nullptr)) != 0) {
    return rc;
  }
  // Write the data.
  if ((rc = dset.write(col.engine_type(TranslationMode::NONE),
                       buf.data(),
                       Dataspace{int (col.nDims() + 1ull),
                           nElements.data(),
                           nElements.data()},
                       std::move(dspace))) == 0) {
    buf.clear(); // Clear the buffer.
  }
  return rc;
}

template <typename COL>
inline
herr_t
hep_hpc::hdf5::NtupleDetail::
flush_one(std::vector<std::string> & buf,
          Dataset & dset,
          COL const & col)
{
  herr_t rc = -1;
  std::vector<char const *> cbuf;
  cbuf.reserve(buf.size());
  std::transform(buf.cbegin(), buf.cend(),
                 std::back_insert_iterator<std::vector<char const *> >(cbuf),
                 [](std::string const & s) { return s.data(); });
  rc = flush_one(cbuf, dset, col);
  if (rc == 0) {
    buf.clear();
  }
  return rc;
}

template <int SZ, typename COL>
inline
herr_t
hep_hpc::hdf5::NtupleDetail::
flush_one(std::vector<std::array<char, SZ> > & buf,
          Dataset & dset,
          COL const & col)
{
  herr_t rc = -1;
  std::vector<char const *> cbuf;
  cbuf.reserve(buf.size());
  std::transform(buf.cbegin(), buf.cend(),
                 std::back_insert_iterator<std::vector<char const *> >(cbuf),
                 [](std::array<char, SZ> const & s) { return s.data(); });
  rc = flush_one(cbuf, dset, col);
  if (rc == 0) {
    buf.clear();
  }
  return rc;
}

template <typename... Args>
void
hep_hpc::hdf5::Ntuple<Args...>::flush()
{
  // No lock here -- lock held by flush_();
  if (flush_(iSequence()) != 0) {
    throw std::runtime_error("HDF5 write failure.");
  }
}

template <size_t I, typename TUPLE, typename COLS, typename... Tail>
inline
void
hep_hpc::hdf5::NtupleDetail::
insert(TUPLE & buffers,
       COLS const & cols,
       typename std::tuple_element<I, COLS>::type::element_type head,
       Tail && ... tail)
{
  insert<I>(buffers, cols, &head, std::forward<Tail>(tail)...);
}

template <size_t I, typename TUPLE, typename COLS, typename... Tail>
inline
void
hep_hpc::hdf5::NtupleDetail::
insert(TUPLE & buffers,
       COLS const & cols,
       typename std::tuple_element<I, COLS>::type::element_type const * head,
       Tail && ... tail)
{
  using std::get;
  auto & col = get<I>(cols);
  auto & buffer = get<I>(buffers);
  if (head != nullptr) {
    buffer.insert(buffer.end(),
                  head,
                  head + col.elementSize());
  } else { // Insert empty
#pragma GCC diagnostic push
#if (defined __GNUC__) && ! GCC_IS_AT_LEAST(5,0,0)
    // Overaggressive warning with GCC < 5.0
    // (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=36750,
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61489).
    _Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"")
#endif
    buffer.insert(buffer.end(), col.elementSize(), {});
#pragma GCC diagnostic pop
  }
  insert<I + 1>(buffers, cols, std::forward<Tail>(tail)...);
}

#endif /* hep_hpc_hdf5_Ntuple_hpp */

// Local Variables:
// mode: c++
// End:
