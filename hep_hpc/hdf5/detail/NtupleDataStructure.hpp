#ifndef hep_hpc_hdf5_detail_NtupleDataStructure_hpp
#define hep_hpc_hdf5_detail_NtupleDataStructure_hpp

#include "hep_hpc/hdf5/Column.hpp"
#include "hep_hpc/hdf5/Group.hpp"
#include "hep_hpc/hdf5/Dataset.hpp"
#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include <string>

namespace hep_hpc {
  namespace hdf5 {
    namespace detail {
      template <typename... Args>
      class NtupleDataStructure;

      Group makeGroup(hid_t file,
                            std::string const & name,
                            bool overwriteContents);

      template <typename COL>
      Dataset makeDataset(hid_t const group, COL const & col, TranslationMode mode);
    }
  }
}

template <typename... Args>
struct hep_hpc::hdf5::detail::NtupleDataStructure {
  NtupleDataStructure(hid_t file, std::string const & name,
                      TranslationMode mode,
                      bool overwriteContents,
                      permissive_column<Args> const & ... cols);

  static constexpr auto nColumns = sizeof...(Args);

  std::tuple<permissive_column<Args>...> columns;
  Group group;
  std::array<Dataset, nColumns> dsets;
};

template <typename... Args>
hep_hpc::hdf5::detail::NtupleDataStructure<Args...>::
NtupleDataStructure(hid_t const file, std::string const & name,
                    TranslationMode mode,
                    bool const overwriteContents,
                    permissive_column<Args> const & ... cols)
  :
  columns(cols...),
  group(makeGroup(file, name, overwriteContents)),
  dsets({makeDataset(group, cols, mode)...})
{
}

template <typename COL>
hep_hpc::hdf5::Dataset
hep_hpc::hdf5::detail::
makeDataset(hid_t const group, COL const & col, TranslationMode mode)
{
  // Cause an exception to be thrown if we have an HDF5 issue.
  ScopedErrorHandler seh(ErrorMode::EXCEPTION);
  dims_t<COL::nDims() + 1ull> dims;
  dims[0] = 0ull;
  std::copy(col.dims(), col.dims() + col.nDims(), std::begin(dims) + 1ull);
  auto maxdims = dims;
  maxdims[0] = H5S_UNLIMITED;
  // Set up creation properties of the dataset.
  PropertyList cprops(H5P_DATASET_CREATE);
  auto chunking = dims;
  chunking[0] = 128ull;
  // Set chunking.
  ErrorController::call(&H5Pset_chunk, cprops, chunking.size(), chunking.data());
  unsigned int const compressionLevel = 6;
  // Set compression level.
  ErrorController::call(&H5Pset_deflate, cprops, compressionLevel);
  // Create and return appropriately constructed dataset.
  return Dataset(group, col.name(), col.engine_type(mode),
                       Dataspace{dims.size(), dims.data(), maxdims.data()},
                       {}, std::move(cprops));
}


#endif /* hep_hpc_hdf5_detail_NtupleDataStructure_hpp */
