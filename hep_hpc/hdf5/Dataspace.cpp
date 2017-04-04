#include "hep_hpc/hdf5/Dataspace.hpp"

hep_hpc::hdf5::Dataspace::
Dataspace(H5S_class_t const classID)
  :
  h5dspace_([&]() { return H5Screate(classID); }, &H5Sclose)
{
}

hep_hpc::hdf5::Dataspace::
Dataspace(hid_t const dspace)
  :
  h5dspace_([&]() { return dspace; }, &H5Sclose)
{
}

hep_hpc::hdf5::Dataspace::
Dataspace(int const rank, hsize_t const * const dims, hsize_t const * const maxdims)
  :
  h5dspace_([&](){return H5Screate_simple(rank, dims, maxdims);}, &H5Sclose)
{
}

hep_hpc::hdf5::Dataspace::
Dataspace(Dataspace const & other)
  :
  h5dspace_([](hid_t id){return H5Scopy(id);}, &H5Sclose, *other.h5dspace_)
{
}

hep_hpc::hdf5::Dataspace &
hep_hpc::hdf5::Dataspace::
operator = (Dataspace const & other)
{
  h5dspace_ = {[](hid_t id){return H5Scopy(id);}, &H5Sclose, *other.h5dspace_};
  return *this;
}
