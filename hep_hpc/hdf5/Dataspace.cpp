#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/Exception.hpp"

namespace {
  void throw_if_invalid_args(hid_t const dspace,
                             hep_hpc::hdf5::ResourceStrategy const strategy)
  {
    if (dspace == H5S_ALL &&
        strategy == hep_hpc::hdf5::ResourceStrategy::handle_tag) {
      throw
        hep_hpc::hdf5::Exception("Dataspace() called with incorrect resource management strategy for H5S_ALL dataspace.");
    }
  }
}


hep_hpc::hdf5::Dataspace::
Dataspace(hid_t const dspace, ResourceStrategy const strategy)
  // Note use of comma operator to detect invalid argument combinations.
  : h5dspace_((throw_if_invalid_args(dspace, strategy),
               (strategy == ResourceStrategy::handle_tag) ?
               Resource(dspace, &H5Sclose) :
               Resource(dspace)))
{
}

