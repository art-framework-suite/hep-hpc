#include "hep_hpc/hdf5/PropertyList.hpp"

hep_hpc::hdf5::PropertyList::
PropertyList(hid_t const propClassID)
  :
  h5plist_([propClassID](){ return H5Pcreate(propClassID); }, &H5Pclose)
{
}

hep_hpc::hdf5::PropertyList::
PropertyList(PropertyList const & other)
  :
  h5plist_([&other](){ return H5Pcopy(*other.h5plist_); }, &H5Pclose)
{
}

hep_hpc::hdf5::PropertyList &
hep_hpc::hdf5::PropertyList::
operator = (PropertyList const & other)
{
  h5plist_ = { [&other](){ return H5Pcopy(*other.h5plist_); }, &H5Pclose };
  return *this;
}

hid_t
hep_hpc::hdf5::PropertyList::
getClass() const
{
  hid_t result = -1;
  hid_t classID = H5Pget_class(*h5plist_);
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
    using namespace std::string_literals;
    using std::to_string;
    throw std::logic_error("INTERNAL ERROR: H5PopertyList::getClass() did not recognize property class ID "s +
                           to_string(classID) + "property list ID "s + to_string(*h5plist_));
  }
  return result;
}
