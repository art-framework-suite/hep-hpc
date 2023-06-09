#ifndef hep_hpc_hdf5_Group_hpp
#define hep_hpc_hdf5_Group_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::Group
//
//   Simple class managing an HDF5 group resource.
//
//   Group will be created by default, but open mode may be specified.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/HID_t.hpp"
#include "hep_hpc/hdf5/Resource.hpp"
#include "hep_hpc/hdf5/ResourceStrategy.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "hdf5.h"

#include <string>

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

  // Adopt an existing HDF5 group ID with the specified management
  // strategy. If observing, caller is responsible for closing at the
  // appropriate time.
  Group(hid_t group, ResourceStrategy strategy);

  // Create or open a group. Property lists may be provided with move
  // semantics to avoid a copy if desired.
  Group(hid_t fileOrGroup,
        std::string const & fullPathName,
        group_mode_t mode = CREATE_MODE,
        PropertyList linkCreationProperties = {},
        PropertyList GroupCreationProperties = {},
        PropertyList GroupAccessProperties = {});

  operator hid_t() const noexcept;

  // Is this a valid group?
  bool is_valid() const noexcept;
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
  static constexpr HID_t INVALID_GROUP_() { return HID_t {};}
  Resource h5group_;
};

inline
hep_hpc::hdf5::Group::
Group(hid_t const group, ResourceStrategy const strategy)
  : h5group_((strategy == ResourceStrategy::handle_tag) ?
             Resource(group, &H5Gclose) :
             Resource(group))
{
}

inline
hep_hpc::hdf5::Group::
operator hid_t() const noexcept
{
  return *h5group_;
}

inline
bool
hep_hpc::hdf5::Group::
is_valid() const noexcept
{
  return *h5group_ > INVALID_GROUP_();
}

inline
hep_hpc::hdf5::Group::
operator bool () const noexcept
{
  return is_valid();
}

inline
H5G_info_t
hep_hpc::hdf5::Group::
info() const
{
  H5G_info_t result;
  // We can't return the error code, so throw if something goes wrong.
  ErrorController::call(ErrorMode::EXCEPTION, &H5Gget_info, *h5group_, &result);
  return result;
}

inline
herr_t
hep_hpc::hdf5::Group::
flush()
{
#if H5_VERS_MAJOR > 1 || (H5_VERS_MAJOR == 1 && H5_VERS_MINOR >= 10)
  return ErrorController::call(&H5Gflush, *h5group_);
#else
  return 0;
#endif
}

inline
herr_t
hep_hpc::hdf5::Group::
refresh()
{
#if H5_VERS_MAJOR > 1 || (H5_VERS_MAJOR == 1 && H5_VERS_MINOR >= 10)
  return ErrorController::call(&H5Grefresh, *h5group_);
#else
  return 0;
#endif
}

inline
void
hep_hpc::hdf5::Group::
reset() {
  h5group_.reset();
}

#endif /* hep_hpc_hdf5_Group_hpp */
