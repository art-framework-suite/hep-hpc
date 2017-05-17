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
//             <dimensions> dims
//             PropertyList linkCreationProperties,
//             PropertyList datasetCreationProperties,
//             PropertyList datasetAccessProperties)
//
// template <typename T>
// Column<T, NDIMS>
// make_scalar_column(std::string name,
//                    PropertyList linkCreationProperties,
//                    PropertyList datasetCreationProperties,
//                    PropertyList datasetAccessProperties)
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/hdf5/Column.hpp"

namespace hep_hpc {
  namespace hdf5 {
    template <typename T, size_t NDIMS = 1>
    Column<T, NDIMS>
    make_column(std::string name,
                typename Column<T, NDIMS>::dims_t dims,
                PropertyList linkCreationProperties = {},
                PropertyList datasetCreationProperties = {},
                PropertyList datasetAccessProperties = {});

    template <typename T>
    Column<T, 1ull>
    make_scalar_column(std::string name,
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

#endif /* hep_hpc_hdf5_make_column_hpp */
