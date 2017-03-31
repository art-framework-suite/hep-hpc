#ifndef hep_hpc_Ntuple_hpp
#define hep_hpc_Ntuple_hpp


#include "hep_hpc/H5File.hpp"
#include "hep_hpc/detail/column.hpp"
#include "hep_hpc/detail/NtupleDataStructure.hpp"

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
  class Ntuple {
public:
    template <typename T> using column = detail::column<T>;
    template <typename T> using permissive_column = detail::permissive_column<T>;

    static constexpr auto nColumns = sizeof...(Args);
    static_assert(nColumns > 0, "Ntuple with zero types is meaningless");

    using name_array = detail::name_array<nColumns>;

    Ntuple(H5File && file,
           std::string const& tablename,
           name_array const& columns,
           bool overwriteContents = false,
           std::size_t bufsize = 1000ull);

    Ntuple(std::string const& filename,
           std::string const& tablename,
           name_array const& columns,
           bool overwriteContents = false,
           std::size_t bufsiz = 1000ull);

    ~Ntuple() noexcept;

    std::string const& name() const { return name_; }

    void insert(Args const & ...);
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
           std::index_sequence<I...>);

    int flush_no_throw();

    template <size_t... I>
    int flush_no_throw_(std::index_sequence<I...>);

    template <size_t I>
    int flush_no_throw_one_();

    std::tuple<std::vector<Args>...> buffers_;
    
    H5File file_;
    std::string name_;
    std::size_t max_;
    std::recursive_mutex mutex_ {};
    detail::NtupleDataStructure<Args...> dd_;
  };

  namespace NtupleDetail {
    H5File verifiedFile(H5File file);

    template <size_t I, typename TUPLE, typename Head, typename... Tail>
    void
    insert(TUPLE & buffers, Head const & head, Tail const & ... tail);

    template <size_t I, typename TUPLE>
    void
    insert(TUPLE &) { }
  } // Namespace NtupleDetail.
} // Namespace hep_hpc.

template <typename... Args>
hep_hpc::Ntuple<Args...>::Ntuple(H5File && file,
                                  std::string const& name,
                                  name_array const& cnames,
                                  bool const overwriteContents,
                                  std::size_t const bufsize) :
  Ntuple{std::move(file), name, cnames, overwriteContents, bufsize, iSequence}
{}

namespace {
  hep_hpc::H5PropertyList fileAccessProperties()
  {
    // Ensure we are using the latest available HDF5 file format to write our data.
    hep_hpc::H5PropertyList plist(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(plist, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    return plist;
  }
}

template <typename... Args>
hep_hpc::Ntuple<Args...>::Ntuple(std::string const& filename,
                                  std::string const& name,
                                  name_array const& cnames,
                                  bool const overwriteContents,
                                  std::size_t const bufsize) :
  Ntuple{H5File(filename, H5F_ACC_TRUNC, {}, fileAccessProperties()),
    name, cnames, overwriteContents, bufsize, iSequence}
{}

template <typename... Args>
template <std::size_t... I>
hep_hpc::Ntuple<Args...>::Ntuple(H5File && file,
                                  std::string const& name,
                                  name_array const& cnames,
                                  bool const overwriteContents,
                                  std::size_t const bufsize,
                                  std::index_sequence<I...>) :
  file_{NtupleDetail::verifiedFile(std::move(file))},
  name_{name},
  max_{bufsize},
  dd_(file_, name, overwriteContents, permissive_column<Args>{cnames[I]}...)
{
  // Reserve buffer space.
  using std::get;
  using swallow = int[];
  swallow {0, (get<I>(buffers_).reserve(bufsize), 0)...};
}

template <typename... Args>
hep_hpc::Ntuple<Args...>::~Ntuple() noexcept
{
  if (flush_no_throw() != 0) {
    std::cerr << "HDF5 failure while flushing.\n";
  }
}

template <typename... Args>
void
hep_hpc::Ntuple<Args...>::insert(Args const & ... args)
{
  using std::get;
  std::lock_guard<decltype(mutex_)> lock {mutex_};
  if (get<0>(buffers_).size() == max_) {
    flush();
  }
  NtupleDetail::insert<0>(buffers_, args...);
}

template <typename... Args>
inline
int
hep_hpc::Ntuple<Args...>::flush_no_throw()
{
  std::lock_guard<decltype(mutex_)> lock {mutex_};
  ScopedErrorHandler seh;
  return flush_no_throw_(iSequence);
}

template <typename... Args>
template <size_t... I>
int
hep_hpc::Ntuple<Args...>::flush_no_throw_(std::index_sequence<I...>)
{
  using std::get;
  auto const results = {flush_no_throw_one_<I>()...};
  return std::any_of(std::cbegin(results), std::cend(results), [](auto const res) { return res != 0; });
}

template <typename... Args>
template <size_t I>
int
hep_hpc::Ntuple<Args...>::
flush_no_throw_one_()
{
  using std::get;
  auto & dset = get<I>(dd_.dsets);
  auto & buf = get<I>(buffers_);
  auto dspace = H5Dataspace{H5Dget_space(dset)};
  // How many dimensions?
  auto const ndims = H5Sget_simple_extent_ndims(dspace);
  // Obtain current and max dimensions.
  std::vector<hsize_t> dims(ndims), maxdims(ndims);
  int rc = H5Sget_simple_extent_dims(dspace, dims.data(), maxdims.data());
  if (rc != ndims) {
    rc = -1;
    return rc;
  }
  auto dimsext = dims;
  hsize_t const nElements = buf.size();
  dimsext[0] = nElements;
  auto const offset = dims[0];
  // Extend long dimension.
  dims[0] += nElements;
  // Update dataset.
  rc = H5Dset_extent(dset, dims.data());
  if (rc != 0) {
    return rc;
  }
  dspace = H5Dataspace{H5Dget_space(dset)};
  // Data selection for write.
  rc = H5Sselect_hyperslab(dspace, H5S_SELECT_SET, &offset, NULL, &nElements, NULL);
  if (rc != 0) {
    return rc;
  }
  H5Dataspace memspace{ndims, dimsext.data(), dimsext.data()};
  // Write the data.
  rc =
    H5Dwrite(dset,
             std::tuple_element<I, decltype(dd_.columns)>::type::engine_type(),
             memspace,
             dspace,
             H5P_DEFAULT,
             buf.data());
  if (rc == 0) {
    buf.clear(); // Clear the buffer.
  }
  return rc;
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

template <size_t I, typename TUPLE, typename Head, typename... Tail>
inline
void
hep_hpc::NtupleDetail::insert(TUPLE & buffers, Head const & head, Tail const & ... tail)
{
  using std::get;
  get<I>(buffers).emplace_back(head);
  insert<I + 1>(buffers, tail...);
}

#endif /* hep_hpc_Ntuple_hpp */

// Local Variables:
// mode: c++
// End:
