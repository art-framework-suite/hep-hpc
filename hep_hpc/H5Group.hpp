#ifndef hep_hpc_H5Group_hpp
#define hep_hpc_H5Group_hpp

#include "hep_hpc/H5PropertyList.hpp"
#include "hep_hpc/HID_t.hpp"
#include "hep_hpc/SimpleRAII.hpp"

#include "hdf5.h"

namespace hep_hpc {
  class H5Group;
}

class hep_hpc::H5Group {
public:
  // Group modes:
  enum group_mode_t : uint8_t { CREATE_MODE, OPEN_MODE, OPEN_OR_CREATE_MODE };

  H5Group() = default;

  // Create or open a group. Note that the H5PropertyList objects (if specified)
  // may be moved in or copied by value, but anyway do not need to live
  // beyond this call.
  H5Group(hid_t fileOrGroup,
          std::string const & fullPathName,
          group_mode_t mode = CREATE_MODE,
          H5PropertyList linkCreationProperties = {},
          H5PropertyList GroupCreationProperties = {},
          H5PropertyList GroupAccessProperties = {});

  operator hid_t() const noexcept;

  explicit operator bool () const noexcept;

  // Obtain information about the group;
  H5G_info_t info() const;

  // Flush the group to disk.
  herr_t flush();

  // Refresh the group from disk.
  herr_t refresh();

  // Reset the group.
  void reset();

private:
  static HID_t const INVALID_GROUP_;
  SimpleRAII<HID_t> h5group_;
};

inline
hep_hpc::H5Group::
operator hid_t() const noexcept
{
  return *h5group_;
}

inline
hep_hpc::H5Group::
operator bool () const noexcept
{
  return *h5group_ > INVALID_GROUP_;
}

inline
H5G_info_t
hep_hpc::H5Group::
info() const
{
  H5G_info_t result;
  (void) H5Gget_info(*h5group_, &result);
  return result;
}

inline
herr_t
hep_hpc::H5Group::
flush()
{
  return H5Gflush(*h5group_);
}

inline
herr_t
hep_hpc::H5Group::
refresh()
{
  return H5Grefresh(*h5group_);
}

inline
void
hep_hpc::H5Group::
reset() {
  h5group_.reset();
}

#endif /* hep_hpc_H5Group_hpp */
