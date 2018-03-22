#ifndef hep_hpc_hdf5_make_column_hpp
#define hep_hpc_hdf5_make_column_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::make_column()
// hep_hpc::hdf5::make_scalar_column()
//
// A pair of function templates to facilitate the in-place construction
// of column description entities (Column) required to describe the
// structure of an hep_hpc::hdf5::Ntuple. Recommended for use,
// especially in conjunction with hep_hpc::hdf5::make_ntuple() as
// described in hep_hpc/hdf5/make_ntuple.hpp. Particularly useful for
// the creation of columns with particular HDF5 properties. The second
// signature is specifically for columns of scalars, where specifying
// the dimensions is unnecessary. Note that in either case, the number and order of
// specification of the properties is: link creation, dataset creation,
// dataset access.
//
// template <typename T, size_t NDIMS = 1>
// Column<T, NDIMS>
// make_column(std::string name,
//             <dimensions> dims,
//             [size_t elementsPerChunk,]
//             std::initializer_list<PropertyList> props = {})
//
// template <typename T>
// Column<T, NDIMS>
// make_scalar_column(std::string name,
//                    [size_t elementsPerChunk,]
//                    std::initializer_list<PropertyList> props = {})
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
                std::initializer_list<PropertyList> props = {});

    template <typename T, size_t NDIMS = 1>
    Column<T, NDIMS>
    make_column(std::string name,
                typename Column<T, NDIMS>::dims_t dims,
                size_t elementsPerChunk,
                std::initializer_list<PropertyList> props = {});

    template <typename T>
    Column<T, 1ull>
    make_scalar_column(std::string name,
                       std::initializer_list<PropertyList> props = {});

    template <typename T>
    Column<T, 1ull>
    make_scalar_column(std::string name,
                       size_t elementsPerChunk,
                       std::initializer_list<PropertyList> props = {});

    namespace detail {
      template <typename COL>
      void setColumnProperties(COL & col,
                               std::initializer_list<PropertyList> props,
                               std::size_t ndims = 0ull,
                               hsize_t * chunking = nullptr);
    }
  }
}

template <typename T, size_t NDIMS>
inline
hep_hpc::hdf5::Column<T, NDIMS>
hep_hpc::hdf5::make_column(std::string name,
                           typename Column<T, NDIMS>::dims_t dims,
                           std::initializer_list<PropertyList> props)
{
  hep_hpc::hdf5::Column<T, NDIMS> result(std::move(name), std::move(dims));
  detail::setColumnProperties(result, std::move(props));
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
                           std::initializer_list<PropertyList> props)
{
  detail::dims_t<NDIMS + 1ull> chunking;
  chunking[0] = elementsPerChunk;
  detail::fill_dims(dims, std::begin(chunking) + 1ull);
  hep_hpc::hdf5::Column<T, NDIMS> result(std::move(name), std::move(dims));
  detail::setColumnProperties(result, std::move(props),
                              chunking.size(), chunking.data());
  return result;
}

template <typename T>
inline
hep_hpc::hdf5::Column<T, 1ull>
hep_hpc::hdf5::make_scalar_column(std::string name,
                                  std::initializer_list<PropertyList> props)
{
  hep_hpc::hdf5::Column<T, 1ull> result(std::move(name), 1ull);
  detail::setColumnProperties(result, std::move(props));
  return result;
}

template <typename T>
inline
hep_hpc::hdf5::Column<T, 1ull>
hep_hpc::hdf5::make_scalar_column(std::string name,
                                  size_t const elementsPerChunk,
                                  std::initializer_list<PropertyList> props)
{
  detail::dims_t<2ull> chunking {elementsPerChunk, 1ull};
  hep_hpc::hdf5::Column<T, 1ull> result(std::move(name), 1ull);
  detail::setColumnProperties(result, std::move(props),
                              chunking.size(), chunking.data());
  return result;
}

template <typename COL>
void
hep_hpc::hdf5::detail::
setColumnProperties(COL & col,
                    std::initializer_list<PropertyList> const props,
                    std::size_t ndims,
                    hsize_t * chunking)
{
  static auto const throwDup =
    [](std::string const & propertyClass) {
    throw std::runtime_error(std::string("setColumnProperties: props contains multiple properties of class ") + propertyClass);
  };
  PropertyList lcp, dcp, dap;
  for (auto && prop : props) {
    if (prop.isClass(H5P_LINK_CREATE)) {
      if (lcp.is_default()) {
        lcp = std::move(prop);
      } else {
        throwDup("H5P_LINK_CREATE");
      }
    } else if (prop.isClass(H5P_DATASET_CREATE)) {
      if (dcp.is_default()) {
        dcp = std::move(prop);
      } else {
        throwDup("H5P_DATASET_CREATE");
      }
    } else if (prop.isClass(H5P_DATASET_ACCESS)) {
      if (dap.is_default()) {
        dap = std::move(prop);
      } else {
        throwDup("H5P_DATASET_ACCESS");
      }
    } else {
      throw std::logic_error("setColumnProperties received a property list of unrecognized class.");
    }
  }
  if (lcp.is_valid_non_default()) {
    col.setLinkCreationProperties(std::move(lcp));
  }
  if (ndims > 0ull) {
    if (dcp.is_default()) {
      dcp = PropertyList(H5P_DATASET_CREATE);
    }
    (void) ErrorController::call(&H5Pset_chunk,
                                 dcp,
                                 ndims,
                                 chunking);
  }
  if (dcp.is_valid_non_default()) {
    col.setDatasetCreationProperties(std::move(dcp));
  }
  if (dap.is_valid_non_default()) {
    col.setDatasetAccessProperties(std::move(dap));
  }
}

#endif /* hep_hpc_hdf5_make_column_hpp */
