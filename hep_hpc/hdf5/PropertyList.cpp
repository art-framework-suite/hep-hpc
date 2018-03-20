#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/Exception.hpp"

#include <stdexcept>

namespace {
  void throw_if_invalid_args(hid_t const plist,
                             hep_hpc::hdf5::ResourceStrategy const strategy)
  {
    if (plist == H5P_DEFAULT &&
        strategy == hep_hpc::hdf5::ResourceStrategy::handle_tag) {
      throw
        hep_hpc::hdf5::Exception("PropertyList() called with incorrect resource management strategy for H5P_DEFAULT property list.");
    }
  }
}

hep_hpc::hdf5::PropertyList::
PropertyList(hid_t const plist, ResourceStrategy const strategy)
  // Note use of comma operator to detect invalid argument combinations.
  : h5plist_((throw_if_invalid_args(plist, strategy),
              (strategy == ResourceStrategy::handle_tag) ?
              Resource(plist, &H5Pclose) :
              Resource(plist)))
{
}

hid_t
hep_hpc::hdf5::PropertyList::
getClass() const
{
  HID_t result;
  hid_t classID = ErrorController::call(&H5Pget_class, *h5plist_);
  if (H5Pequal(classID, H5P_ATTRIBUTE_CREATE)) {
    result = H5P_ATTRIBUTE_CREATE;
  } else if (H5Pequal(classID, H5P_DATASET_ACCESS)) {
    result = H5P_DATASET_ACCESS;
  } else if (H5Pequal(classID, H5P_DATASET_CREATE)) {
    result = H5P_DATASET_CREATE;
  } else if (H5Pequal(classID, H5P_DATASET_XFER)) {
    result = H5P_DATASET_XFER;
  } else if (H5Pequal(classID, H5P_DATATYPE_ACCESS)) {
    result = H5P_DATATYPE_ACCESS;
  } else if (H5Pequal(classID, H5P_DATATYPE_CREATE)) {
    result = H5P_DATATYPE_CREATE;
  } else if (H5Pequal(classID, H5P_FILE_ACCESS)) {
    result = H5P_FILE_ACCESS;
  } else if (H5Pequal(classID, H5P_FILE_CREATE)) {
    result = H5P_FILE_CREATE;
  } else if (H5Pequal(classID, H5P_FILE_MOUNT)) {
    result = H5P_FILE_MOUNT;
  } else if (H5Pequal(classID, H5P_GROUP_ACCESS)) {
    result = H5P_GROUP_ACCESS;
  } else if (H5Pequal(classID, H5P_GROUP_CREATE)) {
    result = H5P_GROUP_CREATE;
  } else if (H5Pequal(classID, H5P_LINK_ACCESS)) {
    result = H5P_LINK_ACCESS;
  } else if (H5Pequal(classID, H5P_LINK_CREATE)) {
    result = H5P_LINK_CREATE;
  } else if (H5Pequal(classID, H5P_OBJECT_COPY)) {
    result = H5P_OBJECT_COPY;
  } else if (H5Pequal(classID, H5P_OBJECT_CREATE)) {
    result = H5P_OBJECT_CREATE;
  } else if (H5Pequal(classID, H5P_STRING_CREATE)) {
    result = H5P_STRING_CREATE;
  } else {
    using std::to_string;
    throw std::logic_error("INTERNAL ERROR: PropertyList::getClass() did not recognize property class ID " +
                           to_string(classID) + "property list ID " + to_string(*h5plist_));
  }
  return result;
}
