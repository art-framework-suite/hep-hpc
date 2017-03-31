#ifndef HHHHSGSHSHJSHGSJ
#define HHHHSGSHSHJSHGSJ

#include "hep_hpc/H5Group.hpp"
#include "hep_hpc/H5Dataset.hpp"
#include "hep_hpc/H5Dataspace.hpp"
#include "hep_hpc/H5PropertyList.hpp"
#include "hep_hpc/ScopedErrorHandler.hpp"

#include <iostream>
#include <string>

namespace hep_hpc {
  namespace detail {
    template <typename... Args>
    class NtupleDataStructure;

    template <int I, typename DSETS, typename T, typename... Args>
    void
    makeDataset(hid_t group,
                DSETS & dsets,
                permissive_column<T> const & col,
                permissive_column<Args> const & ... columns);

    template <int I, typename DSETS>
    void makeDataset(hid_t, DSETS &) { }
  }
}

template <typename... Args>
struct hep_hpc::detail::NtupleDataStructure {
  NtupleDataStructure(hid_t file, std::string const & name,
                      bool overwriteContents,
                      permissive_column<Args> const & ... cols);

  static constexpr auto nColumns = sizeof...(Args);

  std::tuple<permissive_column<Args>...> columns;
  H5Group group;
  std::array<H5Dataset, nColumns> dsets;
};

template <typename... Args>
hep_hpc::detail::NtupleDataStructure<Args...>::
NtupleDataStructure(hid_t const file, std::string const & name,
                    bool const overwriteContents,
                    permissive_column<Args> const & ... cols)
  :
  columns(cols...)
{
  using namespace std::string_literals;
  {
    ScopedErrorHandler seh;
    group = H5Group(file, name);
  }
  if (!group) { // Already exists.
    if (!overwriteContents) {
      std::runtime_error("Group "s + name +
                         " already exists and overwriting is not specified."s);
    }
    if (H5Ldelete(file, name.c_str(), H5P_DEFAULT) != (herr_t)0) {
      std::runtime_error("Group "s + name + " cannot be deleted!"s);
    }
    group = H5Group(file, name);
  }
  makeDataset<0>(group, dsets, cols...);
}

template <int I, typename DSETS, typename T, typename... Args>
void
hep_hpc::detail::
makeDataset(hid_t const group,
            DSETS & dsets,
            permissive_column<T> const & col,
            permissive_column<Args> const & ... columns)
{
  using std::get;
  // FIXME: Should be 1 infinite dim + dims from columns spec when they appear.
  hsize_t const dim = 0ull;
  hsize_t maxdim = H5S_UNLIMITED;
  H5Dataspace dspace{1, &dim, &maxdim};
  // Set up creation properties of the dataset.
  H5PropertyList cprops(H5P_DATASET_CREATE);
  hsize_t const chunking = 128;
  H5Pset_chunk(cprops, 1, &chunking);
  H5Pset_deflate(cprops, 6);
  get<I>(dsets) = H5Dataset(group, col.name(), col.engine_type(), dspace, {}, cprops);

  // Recurse.
  makeDataset<I + 1>(group, dsets, columns...);
}


#endif
