#ifndef hep_hpc_detail_NtupleDataStructure_hpp
#define hep_hpc_detail_NtupleDataStructure_hpp

#include "hep_hpc/hdf5/Group.hpp"
#include "hep_hpc/hdf5/Dataset.hpp"
#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include <string>

namespace hep_hpc {
  namespace detail {
    template <typename... Args>
    class NtupleDataStructure;

    hdf5::Group makeGroup(hid_t file,
                          std::string const & name,
                          bool overwriteContents);

    template <typename COL>
    hdf5::Dataset makeDataset(hid_t const group, COL const & col);
  }
}

template <typename... Args>
struct hep_hpc::detail::NtupleDataStructure {
  NtupleDataStructure(hid_t file, std::string const & name,
                      bool overwriteContents,
                      permissive_column<Args> const & ... cols);

  static constexpr auto nColumns = sizeof...(Args);

  std::tuple<permissive_column<Args>...> columns;
  hdf5::Group group;
  std::array<hdf5::Dataset, nColumns> dsets;
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

hep_hpc::hdf5::Group
hep_hpc::detail::
makeGroup(hid_t file, std::string const & name, bool overwriteContents)
{
  using namespace std::string_literals;
  hdf5::Group group;
  {
    hdf5::ScopedErrorHandler seh;
    group = hdf5::Group(file, name);
  }
  if (!group) { // Already exists.
    if (!overwriteContents) {
      throw std::runtime_error("Group "s + name +
                               " already exists and overwriting is not specified."s);
    }
    if (H5Ldelete(file, name.c_str(), H5P_DEFAULT) != (herr_t)0) {
      throw std::runtime_error("Group "s + name + " cannot be deleted!"s);
    }
    group = hdf5::Group(file, name);
  }
  return group;
}

template <typename COL>
hep_hpc::hdf5::Dataset
hep_hpc::detail::
makeDataset(hid_t const group, COL const & col)
{
  // FIXME: Should be 1 infinite dim + dims from columns spec when they appear.
  hsize_t const dim = 0ull;
  hsize_t maxdim = H5S_UNLIMITED;
  // Set up creation properties of the dataset.
  hdf5::PropertyList cprops(H5P_DATASET_CREATE);
  hsize_t const chunking = 128;
  H5Pset_chunk(cprops, 1, &chunking);
  unsigned int const compressionLevel = 6;
  H5Pset_deflate(cprops, compressionLevel);
  return hdf5::Dataset(group, col.name(), col.engine_type(),
                       hdf5::Dataspace{1, &dim, &maxdim},
                       {}, std::move(cprops));
}


#endif /* hep_hpc_detail_NtupleDataStructure_hpp */
