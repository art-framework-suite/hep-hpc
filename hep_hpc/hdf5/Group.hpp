#ifndef hep_hpc_hdf5_Group_hpp
#define hep_hpc_hdf5_Group_hpp

#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/HID_t.hpp"
#include "hep_hpc/Utilities/SimpleRAII.hpp"

#include "hdf5.h"

namespace hep_hpc {
  namespace hdf5 {
    class Group;
  }
}

class hep_hpc::hdf5::Group {
public:
  // Group modes:
  enum group_mode_t : uint8_t { CREATE_MODE, OPEN_MODE, OPEN_OR_CREATE_MODE };

  Group() = default;

  // Create or open a group. Note that the PropertyList objects (if specified)
  // may be moved in or copied by value, but anyway do not need to live
  // beyond this call.
  Group(hid_t fileOrGroup,
          std::string const & fullPathName,
          group_mode_t mode = CREATE_MODE,
          PropertyList linkCreationProperties = {},
          PropertyList GroupCreationProperties = {},
          PropertyList GroupAccessProperties = {});

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
  detail::SimpleRAII<HID_t> h5group_;
};

inline
hep_hpc::hdf5::Group::
operator hid_t() const noexcept
{
  return *h5group_;
}

inline
hep_hpc::hdf5::Group::
operator bool () const noexcept
{
  return *h5group_ > INVALID_GROUP_;
}

inline
H5G_info_t
hep_hpc::hdf5::Group::
info() const
{
  H5G_info_t result;
  (void) H5Gget_info(*h5group_, &result);
  return result;
}

inline
herr_t
hep_hpc::hdf5::Group::
flush()
{
  return H5Gflush(*h5group_);
}

inline
herr_t
hep_hpc::hdf5::Group::
refresh()
{
  return H5Grefresh(*h5group_);
}

inline
void
hep_hpc::hdf5::Group::
reset() {
  h5group_.reset();
}

#endif /* hep_hpc_hdf5_Group_hpp */
