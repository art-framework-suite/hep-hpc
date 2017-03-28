#ifndef hep_hpc_H5Dataspace_hpp
#define hep_hpc_H5Dataspace_hpp

#include "hep_hpc/SimpleRAII.hpp"

#include "hdf5.h"

#include <iterator>
#include <vector>

namespace hep_hpc {
  class H5Dataspace;

  namespace H5Dataspace_detail {
    template <typename IN_ITER, typename DISTANCE>
    IN_ITER copy_advance(IN_ITER i, DISTANCE n)
    {
      std::advance(i, n);
      return i;
    }
  }
}

class hep_hpc::H5Dataspace {
public:
  H5Dataspace() = default;
  explicit H5Dataspace(H5S_class_t classID);

  H5Dataspace(int rank, hsize_t const * dims, hsize_t const * maxdims);

  template <typename IN_ITER_1, typename IN_ITER_2>
  H5Dataspace(int rank, IN_ITER_1 dim_begin, IN_ITER_2 maxdim_begin);

  template <typename IN_ITER_1, typename IN_ITER_2>
  H5Dataspace(IN_ITER_1 dim_begin, IN_ITER_1 dim_end,
              IN_ITER_2 maxdim_begin, IN_ITER_2 maxdim_end);

  // Copy operations.
  H5Dataspace(H5Dataspace const & other);
  H5Dataspace & operator = (H5Dataspace const & other);

  // Default move operations.
  H5Dataspace(H5Dataspace &&) = default;
  H5Dataspace & operator = (H5Dataspace &&) = default;

  // Is this a valid, non-default dataspace?
  explicit operator bool () const noexcept;

  // Access to the underlying resource handle.
  operator hid_t() const noexcept;

  // Explicitly reset (not usually necessary).
  void reset();

private:
  // Note we are using a plain hid_t here rather than HID_t, because 0
  // (H5S_ALL) is a reasonable default;
  SimpleRAII<hid_t> h5dspace_;
};

template <typename IN_ITER_1, typename IN_ITER_2>
hep_hpc::H5Dataspace::
H5Dataspace(int const rank,
            IN_ITER_1 const dim_begin,
            IN_ITER_2 const maxdim_begin)
  :
  H5Dataspace(dim_begin,
              H5Dataspace_detail::copy_advance(dim_begin, rank),
              maxdim_begin,
              H5Dataspace_detail::copy_advance(maxdim_begin, rank))
{
}

template <typename IN_ITER_1, typename IN_ITER_2>
hep_hpc::H5Dataspace::
H5Dataspace(IN_ITER_1 const dim_begin, IN_ITER_1 const dim_end,
            IN_ITER_2 const maxdim_begin, const IN_ITER_2 maxdim_end)
  :
  h5dspace_([](std::vector<hsize_t> const dims,
               std::vector<hsize_t> const maxdims)
            { return H5Screate_simple(dims.size(), dims.data(), maxdims.data());
            }, &H5Sclose,
            std::vector<hsize_t> {dim_begin, dim_end},
            std::vector<hsize_t> {maxdim_begin, maxdim_end})
{
}

inline
hep_hpc::H5Dataspace::
operator bool () const noexcept
{
  return *h5dspace_ > H5S_ALL;
}

inline
hep_hpc::H5Dataspace::
operator hid_t() const noexcept
{
  return *h5dspace_;
}

inline
void
hep_hpc::H5Dataspace::
reset()
{
  h5dspace_.reset();
}
#endif /* hep_hpc_H5Dataspace_hpp */
