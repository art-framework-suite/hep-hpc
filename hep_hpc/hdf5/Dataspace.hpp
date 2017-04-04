#ifndef hep_hpc_hdf5_Dataspace_hpp
#define hep_hpc_hdf5_Dataspace_hpp

#include "hep_hpc/Utilities/SimpleRAII.hpp"

#include "hdf5.h"

#include <iterator>
#include <vector>

namespace hep_hpc {
  namespace hdf5 {
    class Dataspace;

    namespace Dataspace_detail {
      template <typename IN_ITER, typename DISTANCE>
      IN_ITER copy_advance(IN_ITER i, DISTANCE n)
      {
        std::advance(i, n);
        return i;
      }
    }
  }
}

class hep_hpc::hdf5::Dataspace {
public:
  Dataspace() = default;
  explicit Dataspace(H5S_class_t classID);

  // Take ownership.
  explicit Dataspace(hid_t dspace);

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
  // Note we are using a plain hid_t here rather than HID_t, because 0
  // (H5S_ALL) is a reasonable default;
  detail::SimpleRAII<hid_t> h5dspace_;
};

template <typename IN_ITER_1>
hep_hpc::hdf5::Dataspace::
Dataspace(int const rank,
            IN_ITER_1 const dim_begin)
  :
  Dataspace(dim_begin, Dataspace_detail::copy_advance(dim_begin, rank))
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
}

template <typename IN_ITER_1, typename IN_ITER_2>
hep_hpc::hdf5::Dataspace::
Dataspace(int const rank,
            IN_ITER_1 const dim_begin,
            IN_ITER_2 const maxdim_begin)
  :
  Dataspace(dim_begin,
              Dataspace_detail::copy_advance(dim_begin, rank),
              maxdim_begin,
              Dataspace_detail::copy_advance(maxdim_begin, rank))
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
}

inline
hep_hpc::hdf5::Dataspace::
operator bool () const noexcept
{
  return *h5dspace_ > H5S_ALL;
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
