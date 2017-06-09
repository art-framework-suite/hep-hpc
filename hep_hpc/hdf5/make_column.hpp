#ifndef hep_hpc_hdf5_make_column_hpp
#define hep_hpc_hdf5_make_column_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::make_column()
// hep_hpc::hdf5::make_scalar_column()
//
// A pair of function templates to facilitate the in-place
// construction of column description entities (Column) required to
// describe the structure of an hep_hpc::hdf5::Ntuple. Recommended for
// use, especially in conjunction with hep_hpc::hdf5::make_ntuple() as
// described in hep_hpc/hdf5/make_ntuple.hpp. Particularly useful for
// the creation of columns with particular HDF5 properties. The second
// signature is specifically for columns of scalars, where specifying
// the dimensions is unnecessary.
//
// template <typename T, size_t NDIMS = 1>
// Column<T, NDIMS>
// make_column(std::string name,
//             <dimensions> dims,
//             [size_t elementsPerChunk,]
//             PropertyList linkCreationProperties = {},
//             PropertyList datasetCreationProperties = {},
//             PropertyList datasetAccessProperties = {})
//
// template <typename T>
// Column<T, NDIMS>
// make_scalar_column(std::string name,
//                    [size_t elementsPerChunk,]
//                    PropertyList linkCreationProperties = {},
//                    PropertyList datasetCreationProperties = {},
//                    PropertyList datasetAccessProperties = {})
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/hdf5/Column.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"

#include "hdf5.h"

namespace hep_hpc {
  namespace hdf5 {
    template <typename T, size_t NDIMS = 1>
    Column<T, NDIMS>
    make_column(std::string name,
                typename Column<T, NDIMS>::dims_t dims,
                PropertyList linkCreationProperties = {},
                PropertyList datasetCreationProperties = {},
                PropertyList datasetAccessProperties = {});

    template <typename T, size_t NDIMS = 1>
    Column<T, NDIMS>
    make_column(std::string name,
                typename Column<T, NDIMS>::dims_t dims,
                size_t elementsPerChunk,
                PropertyList linkCreationProperties = {},
                PropertyList datasetCreationProperties = {},
                PropertyList datasetAccessProperties = {});

    template <typename T>
    Column<T, 1ull>
    make_scalar_column(std::string name,
                       PropertyList linkCreationProperties = {},
                       PropertyList datasetCreationProperties = {},
                       PropertyList datasetAccessProperties = {});

    template <typename T>
    Column<T, 1ull>
    make_scalar_column(std::string name,
                       size_t elementsPerChunk,
                       PropertyList linkCreationProperties = {},
                       PropertyList datasetCreationProperties = {},
                       PropertyList datasetAccessProperties = {});
  }
}

template <typename T, size_t NDIMS>
inline
hep_hpc::hdf5::Column<T, NDIMS>
hep_hpc::hdf5::make_column(std::string name,
                           typename Column<T, NDIMS>::dims_t dims,
                           PropertyList linkCreationProperties,
                           PropertyList datasetCreationProperties,
                           PropertyList datasetAccessProperties)
{
  hep_hpc::hdf5::Column<T, NDIMS>
    result(std::move(name), std::move(dims));
  result.setLinkCreationProperties(std::move(linkCreationProperties));
  result.setDatasetCreationProperties(std::move(datasetCreationProperties));
  result.setDatasetAccessProperties(std::move(datasetAccessProperties));
  return result;
}

namespace hep_hpc {
  namespace hdf5 {
    namespace detail {
      template <size_t NDIMS, typename OutIter>
      void
      fill_dims(dims_t<NDIMS> const & dims,
                OutIter const oIter)
      {
        std::copy(std::begin(dims),
                  std::begin(dims) + NDIMS,
                  oIter);
      }

      template <typename OutIter>
      void
      fill_dims(size_t const dim,
                OutIter const oIter)
      {
        *oIter = dim;
      }
    }
  }
}

template <typename T, size_t NDIMS>
inline
hep_hpc::hdf5::Column<T, NDIMS>
hep_hpc::hdf5::make_column(std::string name,
                           typename Column<T, NDIMS>::dims_t dims,
                           size_t const elementsPerChunk,
                           PropertyList linkCreationProperties,
                           PropertyList datasetCreationProperties,
                           PropertyList datasetAccessProperties)
{
  detail::dims_t<NDIMS + 1ull> chunking;
  chunking[0] = elementsPerChunk;
  detail::fill_dims(dims, std::begin(chunking) + 1ull);
  (void) ErrorController::call(&H5Pset_chunk,
                               datasetCreationProperties,
                               chunking.size(),
                               chunking.data());
  return make_column<T, NDIMS>(std::move(name),
                               std::move(dims),
                               std::move(linkCreationProperties),
                               std::move(datasetCreationProperties),
                               std::move(datasetAccessProperties));
}

template <typename T>
inline
hep_hpc::hdf5::Column<T, 1ull>
hep_hpc::hdf5::make_scalar_column(std::string name,
                                  PropertyList linkCreationProperties,
                                  PropertyList datasetCreationProperties,
                                  PropertyList datasetAccessProperties)
{
  hep_hpc::hdf5::Column<T, 1ull>
    result(std::move(name), 1ull);
  result.setLinkCreationProperties(std::move(linkCreationProperties));
  result.setDatasetCreationProperties(std::move(datasetCreationProperties));
  result.setDatasetAccessProperties(std::move(datasetAccessProperties));
  return result;
}

template <typename T>
inline
hep_hpc::hdf5::Column<T, 1ull>
hep_hpc::hdf5::make_scalar_column(std::string name,
                                  size_t const elementsPerChunk,
                                  PropertyList linkCreationProperties,
                                  PropertyList datasetCreationProperties,
                                  PropertyList datasetAccessProperties)
{
  detail::dims_t<2ull> chunking {elementsPerChunk, 1ull};
  (void) ErrorController::call(&H5Pset_chunk,
                               datasetCreationProperties,
                               chunking.size(),
                               chunking.data());
  return make_scalar_column<T>(std::move(name),
                               std::move(linkCreationProperties),
                               std::move(datasetCreationProperties),
                               std::move(datasetAccessProperties));
}

#endif /* hep_hpc_hdf5_make_column_hpp */
