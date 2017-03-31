#ifndef hep_hpc_detail_NtupleDataStructure_hpp
#define hep_hpc_detail_NtupleDataStructure_hpp

#include "hep_hpc/H5Group.hpp"
#include "hep_hpc/H5Dataset.hpp"
#include "hep_hpc/H5Dataspace.hpp"
#include "hep_hpc/H5PropertyList.hpp"
#include "hep_hpc/ScopedErrorHandler.hpp"

#include <string>

namespace hep_hpc {
  namespace detail {
    template <typename... Args>
    class NtupleDataStructure;

    H5Group makeGroup(hid_t file,
                      std::string const & name,
                      bool overwriteContents);

    template <typename COL>
    H5Dataset makeDataset(hid_t const group, COL const & col);
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
  columns(cols...),
  group(detail::makeGroup(file, name, overwriteContents)),
  dsets({detail::makeDataset(group, cols)...})
{
}

hep_hpc::H5Group
hep_hpc::detail::
makeGroup(hid_t file, std::string const & name, bool overwriteContents)
{
  using namespace std::string_literals;
  H5Group group;
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
  return group;
}

template <typename COL>
hep_hpc::H5Dataset
hep_hpc::detail::
makeDataset(hid_t const group, COL const & col)
{
  // FIXME: Should be 1 infinite dim + dims from columns spec when they appear.
  hsize_t const dim = 0ull;
  hsize_t maxdim = H5S_UNLIMITED;
  H5Dataspace dspace{1, &dim, &maxdim};
  // Set up creation properties of the dataset.
  H5PropertyList cprops(H5P_DATASET_CREATE);
  hsize_t const chunking = 128;
  H5Pset_chunk(cprops, 1, &chunking);
  H5Pset_deflate(cprops, 6);
  return H5Dataset(group, col.name(), col.engine_type(), dspace, {}, cprops);
}


#endif /* hep_hpc_detail_NtupleDataStructure_hpp */
