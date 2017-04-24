#ifndef hep_hpc_Ntuple_hpp
#define hep_hpc_Ntuple_hpp
////////////////////////////////////////////////////////////////////////
// Ntuple.hpp
//
// An implementation of an Ntuple with an HDF5 backend.
//
////////////////////////////////////
// Overview.
//
// * This is a variadic template (takes an arbitrary number of template
//   arguments).
// * Each template argument represents a column of the Ntuple.
// * Insertions are row-wise, storage is column-wise.
//
////////////////////////////////////
// Interface
//
// Template arguments are specified as basic types (double, int, etc.)
// or of type hep_hpc::Column (see hep_hpc/Column.hpp for
// details). Currently simple n-dimensional fixed-size arrays of
// arithmetic types are supported (no string types as yet).
//
////////////////////////////////////
//
// Ntuple<Args...>(hid_t file,
//                 std::string tablename,
//                 column_info_t columns,
//                 [TranslationMode mode,]
//                 bool overwriteContents,
//                 std::size_t bufsize); // or ...
//
// Ntuple<Args...>(std::string filename,
//                 std::string tablename,
//                 column_info_t columns,
//                 [TranslationMode mode,]
//                 std::size_t bufsize);
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
//   thrown currently).
//
//   Buffer size controls how many rows are cached in memory before
//   being flushed to the file; defaults to 1000 if not specified.
//
//   Insertion is row-wise; storage is column-wise.
//
////////////////////////////////////
//
// std::string name() const;
//
//   Return the name of the group containing the Ntuple data.
//
////////////////////////////////////
//
// void insert(ELEMENT<Args> const *...);
//
//   Insert a row of data. Each argument is expected to be a pointer to
//   the basic element type T of each column. If the argument is not
//   nullptr, it is expected to be a pointer to a contiguous sequence of
//   items of the column's basic type (e.g. double) of length
//   Column::elementSize(). If the argument is nullptr, then the buffer
//   will be filled with Column::elementSize() default-constructed items
//   of type T. This contiguous sequence must be organized according to
//   the HDF5 description for n-dimensional array representation:
//   right-most index moves fastest.
//
//   N.B. Strings are supported with a basic element type of const char
//   * (or char *). One therefore insert()s providing a char const * * (
//   or char * *) which points to a contiguous array of char const * (or
//   char *), each of which must continue to point to valid
//   null-terminated character strings until the buffer has been
//   flushed.
//
//   If the buffer is full, flush it first.
//
////////////////////////////////////
//
// void flush()
//
//   Flush the currently-buffered data to file.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/Column.hpp"
#include "hep_hpc/detail/NtupleDataStructure.hpp"
#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "hdf5.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace hep_hpc {
  template <typename... Args>
  class Ntuple;

} // Namespace hep_hpc.

template <typename... Args>
class hep_hpc::Ntuple {
public:
  static constexpr auto nColumns() { return sizeof...(Args); }

  using column_info_t = std::tuple<detail::permissive_column<Args>...>;

  template <typename T>
  using Element_t = typename detail::permissive_column<T>::element_type;

  Ntuple(hid_t file,
         std::string tablename,
         column_info_t columns,
         bool overwriteContents = false,
         std::size_t bufsize = 1000ull);

  Ntuple(hid_t file,
         std::string tablename,
         column_info_t columns,
         TranslationMode mode,
         bool overwriteContents = false,
         std::size_t bufsize = 1000ull);

  Ntuple(std::string filename,
         std::string tablename,
         column_info_t columns,
         std::size_t bufsiz = 1000ull);

  Ntuple(std::string filename,
         std::string tablename,
         column_info_t columns,
         TranslationMode mode,
         std::size_t bufsiz = 1000ull);

  ~Ntuple() noexcept;

  std::string const& name() const { return name_; }

  void insert(Element_t<Args> const * ...);
  void flush();

  // Disable copying
  Ntuple(Ntuple const&) = delete;
  Ntuple& operator=(Ntuple const&) = delete;

private:
  static_assert(nColumns() > 0, "Ntuple with zero types is meaningless");

  static constexpr auto iSequence()
    { return std::make_index_sequence<nColumns()>(); }

  // This is the c'tor that does all of the work. It exists so that the
  // Args... and column-names array can be expanded in parallel.
  template <std::size_t... I>
  Ntuple(hdf5::File file,
         std::string tablename,
         column_info_t columns,
         TranslationMode mode,
         bool overwriteContents,
         std::size_t bufsize,
         std::index_sequence<I...>);

  template <size_t... I>
  int flush_(std::index_sequence<I...>);

  std::tuple<std::vector<Element_t<Args> >...> buffers_;
    
  hdf5::File file_;
  std::string name_;
  std::array<size_t, nColumns()> max_;
  std::recursive_mutex mutex_ {};
  detail::NtupleDataStructure<Args...> dd_;
};

////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////

namespace hep_hpc {
  namespace NtupleDetail {
    hdf5::File verifiedFile(hdf5::File file);

    template <size_t I, typename TUPLE, typename COLS,
              typename Head, typename... Tail>
    void
    insert(TUPLE & buffers, COLS const & cols,
           Head const * head, Tail const * ... tail);

    template <size_t I, typename TUPLE, typename COLS>
    void
    insert(TUPLE &, COLS const &) { }

    template <typename BUFFER, typename COL>
    int flush_one(BUFFER & buf, hid_t dset, COL const & col);
  } // Namespace NtupleDetail.
} // Namespace hep_hpc.

template <typename... Args>
hep_hpc::Ntuple<Args...>::Ntuple(hid_t file,
                                  std::string name,
                                  column_info_t columns,
                                  bool const overwriteContents,
                                  std::size_t const bufsize) :
  Ntuple{hdf5::File(file),
    std::move(name),
    std::move(columns),
    TranslationMode::NONE,
    overwriteContents,
    bufsize}
{}

template <typename... Args>
hep_hpc::Ntuple<Args...>::Ntuple(hid_t file,
                                  std::string name,
                                  column_info_t columns,
                                  TranslationMode mode,
                                  bool const overwriteContents,
                                  std::size_t const bufsize) :
  Ntuple{hdf5::File(file),
    std::move(name),
    std::move(columns),
    mode,
    overwriteContents,
    bufsize}
{}

namespace {
  hep_hpc::hdf5::PropertyList fileAccessProperties()
  {
    // Ensure we are using the latest available HDF5 file format to write our data.
    hep_hpc::hdf5::PropertyList plist(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(plist, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    return plist;
  }
}

template <typename... Args>
hep_hpc::Ntuple<Args...>::Ntuple(std::string filename,
                                  std::string name,
                                  column_info_t columns,
                                  std::size_t const bufsize) :
  Ntuple{hdf5::File(std::move(filename), H5F_ACC_TRUNC, {}, fileAccessProperties()),
    std::move(name), std::move(columns), TranslationMode::NONE, false, bufsize, iSequence()}
{}

template <typename... Args>
hep_hpc::Ntuple<Args...>::Ntuple(std::string filename,
                                  std::string name,
                                  column_info_t columns,
                                  TranslationMode mode,
                                  std::size_t const bufsize) :
  Ntuple{hdf5::File(std::move(filename), H5F_ACC_TRUNC, {}, fileAccessProperties()),
    std::move(name), std::move(columns), mode, false, bufsize, iSequence()}
{}

template <typename... Args>
template <std::size_t... I>
hep_hpc::Ntuple<Args...>::Ntuple(hdf5::File file,
                                  std::string name,
                                  column_info_t columns,
                                  TranslationMode mode,
                                  bool const overwriteContents,
                                  std::size_t const bufsize,
                                  std::index_sequence<I...>) :
  file_{NtupleDetail::verifiedFile(std::move(file))},
  name_{std::move(name)},
  max_{(std::get<I>(columns).elementSize() * bufsize)...},
  dd_(file_, name_, mode, overwriteContents, std::move(std::get<I>(columns))...)
{
  // Reserve buffer space.
  using swallow = int[];
  // Reserve the right amount of space in each buffer.
  swallow {0, (std::get<I>(buffers_).reserve(max_[I]), 0)...};
}

template <typename... Args>
hep_hpc::Ntuple<Args...>::~Ntuple() noexcept
{
  hdf5::ScopedErrorHandler seh(hdf5::ErrorMode::HDF5_DEFAULT);
  if (flush_(iSequence()) != 0) {
    std::cerr << "HDF5 failure while flushing.\n";
  }
}

template <typename... Args>
void
hep_hpc::Ntuple<Args...>::insert(Element_t<Args> const * ... args)
{
  using std::get;
  std::lock_guard<decltype(mutex_)> lock {mutex_};
  if (get<0>(buffers_).size() == max_[0]) {
    flush();
  }
  NtupleDetail::insert<0>(buffers_, dd_.columns, args...);
}

template <typename... Args>
template <size_t... I>
int
hep_hpc::Ntuple<Args...>::flush_(std::index_sequence<I...>)
{
  using std::get;
  std::lock_guard<decltype(mutex_)> lock {mutex_};
  auto const results =
    {0, NtupleDetail::flush_one(get<I>(buffers_),
                                get<I>(dd_.dsets),
                                get<I>(dd_.columns))...};
  return std::any_of(std::cbegin(results),
                     std::cend(results),
                     [](auto const res) { return res != 0; });
}

template <typename BUFFER, typename COL>
int
hep_hpc::NtupleDetail::
flush_one(BUFFER & buf, hid_t dset, COL const & col)
{
  using std::get;
  using hdf5::ErrorController;
  herr_t rc = -1;
  // Obtain the current dataspace for this dataset.
  auto dspace = hdf5::Dataspace{ErrorController::call(&H5Dget_space, dset)};
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
  dspace = hdf5::Dataspace{ErrorController::call(&H5Dget_space, dset)};
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
  if ((rc = ErrorController::call(&H5Dwrite,
                                  dset,
                                  // Memory type should be native.
                                  col.engine_type(TranslationMode::NONE),
                                  hdf5::Dataspace{col.nDims() + 1ull,
                                      nElements.data(),
                                      nElements.data()},
                                  std::move(dspace),
                                  H5P_DEFAULT,
                                  buf.data())) == 0) {
    buf.clear(); // Clear the buffer.
  }
  return rc;
}

template <typename... Args>
void
hep_hpc::Ntuple<Args...>::flush()
{
  // No lock here -- lock held by flush_();
  if (flush_(iSequence()) != 0) {
    throw std::runtime_error("HDF5 write failure.");
  }
}

template <size_t I, typename TUPLE, typename COLS,
          typename Head, typename... Tail>
inline
void
hep_hpc::NtupleDetail::insert(TUPLE & buffers,
                               COLS const & cols,
                               Head const * head,
                               Tail const * ... tail)
{
  using std::get;
  auto & col = get<I>(cols);
  auto & buffer = get<I>(buffers);
  if (head != nullptr) {
    buffer.insert(buffer.end(),
                  head,
                  head + col.elementSize());
  } else { // Insert empty
    buffer.insert(buffer.end(), col.elementSize(), {});
  }
  insert<I + 1>(buffers, cols, tail...);
}

#endif /* hep_hpc_Ntuple_hpp */

// Local Variables:
// mode: c++
// End:
