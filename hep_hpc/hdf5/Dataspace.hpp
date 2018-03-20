#ifndef hep_hpc_hdf5_Dataspace_hpp
#define hep_hpc_hdf5_Dataspace_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::Dataspace
//
// Class representing an HDF5 dataspace, with resource management.
//
// N.B. Dataspaces do not generally need to last beyond the call
//      passing them to an HDF5 function.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/Utilities/detail/copy_advance.hpp"
#include "hep_hpc/hdf5/Resource.hpp"
#include "hep_hpc/hdf5/ResourceStrategy.hpp"

#include "hdf5.h"

#include <iterator>
#include <vector>

namespace hep_hpc {
  namespace hdf5 {
    class Dataspace;
  }
}

class hep_hpc::hdf5::Dataspace {
public:
  Dataspace() = default;
  explicit Dataspace(H5S_class_t classID);

  // Adopt an existing HDF5 dataspace ID with the specified management
  // strategy. If observing, caller is responsible for closing at the
  // appropriate time.
  explicit Dataspace(hid_t dspace,
                     ResourceStrategy strategy = ResourceStrategy::handle_tag);

  // Basic
  Dataspace(int rank, hsize_t const * dims, hsize_t const * maxdims = nullptr);

  // No max dims.
  template <typename IN_ITER_1>
  Dataspace(int rank, IN_ITER_1 dim_begin);

  template <typename IN_ITER_1>
  Dataspace(IN_ITER_1 dim_begin, IN_ITER_1 dim_end);

  // With max dims.
  template <typename IN_ITER_1, typename IN_ITER_2>
  Dataspace(int rank, IN_ITER_1 dim_begin, IN_ITER_2 maxdim_begin);

  template <typename IN_ITER_1, typename IN_ITER_2>
  Dataspace(IN_ITER_1 dim_begin, IN_ITER_1 dim_end,
            IN_ITER_2 maxdim_begin, IN_ITER_2 maxdim_end);

  // Copy operations.
  Dataspace(Dataspace const & other);
  Dataspace & operator = (Dataspace const & other);

  // Default move operations.
  Dataspace(Dataspace &&) = default;
  Dataspace & operator = (Dataspace &&) = default;

  // Is this a valid, non-default dataspace?
  explicit operator bool () const noexcept;

  // Access to the underlying resource handle.
  operator hid_t() const noexcept;

  // Explicitly reset (not usually necessary).
  void reset();

private:
  static constexpr HID_t const INVALID_DSPACE_ {};
  // Note 0 (H5S_ALL) is a reasonable default;
  Resource h5dspace_ {0};
};

inline
hep_hpc::hdf5::Dataspace::
Dataspace(H5S_class_t const classID)
  :
  h5dspace_(&H5Screate, &H5Sclose, classID)
{
  if (*h5dspace_ < 0) { // Error that we didn't throw over.
    h5dspace_.release();
  }
}

inline
hep_hpc::hdf5::Dataspace::
Dataspace(int const rank,
          hsize_t const * const dims,
          hsize_t const * const maxdims)
  :
  h5dspace_(&H5Screate_simple, &H5Sclose, rank, dims, maxdims)
{
  if (*h5dspace_ < 0) { // Error that we didn't throw over.
    h5dspace_.release();
  }
}

template <typename IN_ITER_1>
hep_hpc::hdf5::Dataspace::
Dataspace(int const rank, IN_ITER_1 const dim_begin)
  :
  Dataspace(dim_begin, hep_hpc::detail::copy_advance(dim_begin, rank))
{
}

template <typename IN_ITER_1>
hep_hpc::hdf5::Dataspace::
Dataspace(IN_ITER_1 const dim_begin, IN_ITER_1 const dim_end)
  :
  h5dspace_([](std::vector<hsize_t> const dims)
            { return H5Screate_simple(dims.size(), dims.data(), nullptr);
            }, &H5Sclose,
            std::vector<hsize_t> {dim_begin, dim_end})
{
  if (*h5dspace_ < 0) { // Error that we didn't throw over.
    h5dspace_.release();
  }
}

template <typename IN_ITER_1, typename IN_ITER_2>
hep_hpc::hdf5::Dataspace::
Dataspace(int const rank,
          IN_ITER_1 const dim_begin,
          IN_ITER_2 const maxdim_begin)
  :
  Dataspace(dim_begin,
            hep_hpc::detail::copy_advance(dim_begin, rank),
            maxdim_begin,
            hep_hpc::detail::copy_advance(maxdim_begin, rank))
{
}

template <typename IN_ITER_1, typename IN_ITER_2>
hep_hpc::hdf5::Dataspace::
Dataspace(IN_ITER_1 const dim_begin, IN_ITER_1 const dim_end,
          IN_ITER_2 const maxdim_begin, const IN_ITER_2 maxdim_end)
  :
  h5dspace_([](std::vector<hsize_t> const dims,
               std::vector<hsize_t> const maxdims)
            { return H5Screate_simple(dims.size(), dims.data(), maxdims.data());
            }, &H5Sclose,
            std::vector<hsize_t> {dim_begin, dim_end},
            std::vector<hsize_t> {maxdim_begin, maxdim_end})
{
  if (*h5dspace_ < 0) { // Error that we didn't throw over.
    h5dspace_.release();
  }
}

inline
hep_hpc::hdf5::Dataspace::
Dataspace(Dataspace const & other)
  :
  h5dspace_(&H5Scopy, &H5Sclose, *other.h5dspace_)
{
  if (*h5dspace_ < 0) { // Error that we didn't throw over.
    h5dspace_.release();
  }
}

inline
hep_hpc::hdf5::Dataspace &
hep_hpc::hdf5::Dataspace::
operator = (Dataspace const & other)
{
  h5dspace_ = {&H5Scopy, &H5Sclose, *other.h5dspace_};
  if (*h5dspace_ < 0) { // Error that we didn't throw over.
    h5dspace_.release();
  }
  return *this;
}

inline
hep_hpc::hdf5::Dataspace::
operator bool () const noexcept
{
  return *h5dspace_ > INVALID_DSPACE_;
}

inline
hep_hpc::hdf5::Dataspace::
operator hid_t() const noexcept
{
  return *h5dspace_;
}

inline
void
hep_hpc::hdf5::Dataspace::
reset()
{
  h5dspace_.reset();
}
#endif /* hep_hpc_hdf5_Dataspace_hpp */
