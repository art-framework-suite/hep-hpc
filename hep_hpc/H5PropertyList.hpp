#ifndef hep_hpc_H5PropertyList_hpp
#define hep_hpc_H5PropertyList_hpp

#include "hep_hpc/SimpleRAII.hpp"

#include "hdf5.h"

namespace hep_hpc {
  class H5PropertyList;
}

class hep_hpc::H5PropertyList {
public:
  H5PropertyList() = default;

  explicit H5PropertyList(hid_t propClassID);

  // Copy operations.
  H5PropertyList(H5PropertyList const & other);
  H5PropertyList & operator = (H5PropertyList const & other);

  // Default move operations.
  H5PropertyList(H5PropertyList &&) = default;
  H5PropertyList & operator = (H5PropertyList &&) = default;

  // Is this a valid, non-default property list?
  explicit operator bool () const noexcept;

  // Access to the underlying resource handle.
  operator hid_t() const noexcept;

  // What is the class of this property list?
  hid_t getClass() const;
  std::string getClassName() const;
  bool isClass(hid_t propClassID) const;

  // Explicitly finish with this property list, cleaning up (not usually
  // necessary).
  void reset();

private:
  // Note we are using a plain hid_t here rather than HID_t, because 0
  // (H5P_DEFAULT) is a reasonable default;
  SimpleRAII<hid_t> h5plist_;
};

inline
hep_hpc::H5PropertyList::
operator bool () const noexcept
{
  return *h5plist_ > H5P_DEFAULT;
}

inline
hep_hpc::H5PropertyList::
operator hid_t() const noexcept
{
  return *h5plist_;
}

inline
std::string
hep_hpc::H5PropertyList::
getClassName() const {
  // Return from H5Pget_class_name() must be memory-managed.
  SimpleRAII<char *> cname(&H5Pget_class_name, &free, H5Pget_class(*h5plist_));
  return *cname;
}

inline
bool
hep_hpc::H5PropertyList::
isClass(hid_t propClassID) const
{
  return H5Pequal(H5Pget_class(*h5plist_), propClassID);
}

inline
void
hep_hpc::H5PropertyList::
reset() {
  h5plist_.reset();
}

#endif /* hep_hpc_H5PropertyList_hpp */
