#include "hep_hpc/H5Dataspace.hpp"

hep_hpc::H5Dataspace::
H5Dataspace(H5S_class_t const classID)
  :
  h5dspace_([&]() { return H5Screate(classID); }, &H5Sclose)
{
}

hep_hpc::H5Dataspace::
H5Dataspace(hid_t const dspace)
  :
  h5dspace_([&]() { return dspace; }, &H5Sclose)
{
}

hep_hpc::H5Dataspace::
H5Dataspace(int const rank, hsize_t const * const dims, hsize_t const * const maxdims)
  :
  h5dspace_([&](){return H5Screate_simple(rank, dims, maxdims);}, &H5Sclose)
{
}

hep_hpc::H5Dataspace::
H5Dataspace(H5Dataspace const & other)
  :
  h5dspace_([](hid_t id){return H5Scopy(id);}, &H5Sclose, *other.h5dspace_)
{
}

hep_hpc::H5Dataspace &
hep_hpc::H5Dataspace::
operator = (H5Dataspace const & other)
{
  h5dspace_ = {[](hid_t id){return H5Scopy(id);}, &H5Sclose, *other.h5dspace_};
  return *this;
}
