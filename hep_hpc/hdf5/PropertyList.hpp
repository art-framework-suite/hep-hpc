#ifndef hep_hpc_hdf5_PropertyList_hpp
#define hep_hpc_hdf5_PropertyList_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::PropertyList.
//
// Class representing an HDF5 property list, with resource management.
//
// N.B. Property lists do not generally need to last beyond the call
//      passing them to an HDF5 function.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/Utilities/SimpleRAII.hpp"
#include "hep_hpc/hdf5/Resource.hpp"
#include "hep_hpc/hdf5/ResourceStrategy.hpp"

#include "hdf5.h"

#include <cstdlib>
#include <string>

namespace hep_hpc {
  namespace hdf5 {
    class PropertyList;
  }
}

class hep_hpc::hdf5::PropertyList {
public:
  PropertyList() = default;

  // Create a defaulted property list of the specified class.
  explicit PropertyList(hid_t propClassID);

  // Adopt an existing HDF5 property list ID with the specified
  // management strategy. If observing, caller is responsible for
  // closing at the appropriate time.
  PropertyList(hid_t plist_id, ResourceStrategy strategy);

  // Copy operations.
  PropertyList(PropertyList const & other);
  PropertyList & operator = (PropertyList const & other);

  // Default move operations.
  PropertyList(PropertyList &&) = default;
  PropertyList & operator = (PropertyList &&) = default;

  // Is this a valid property list?
  bool is_valid() const noexcept;
  explicit operator bool () const noexcept;

  // Is this a default property list?
  bool is_default() const noexcept;

  // Is this a valid, non-default property list?
  bool is_valid_non_default() const noexcept;

  // Access to the underlying resource handle.
  operator hid_t () const noexcept;

  // Chaining operator
  template <typename FUNC, typename... Args>
  PropertyList & operator () (FUNC func, Args && ... args);

  // What is the class of this property list?
  hid_t getClass() const;
  std::string getClassName() const;
  bool isClass(hid_t propClassID) const;

  // Explicitly finish with this property list, cleaning up (not usually
  // necessary).
  void reset();

private:
  static constexpr HID_t INVALID_PLIST_() { return HID_t {};}
  // Note H5P_DEFAULT is a reasonable default: no memory management
  // required.
  Resource h5plist_ {H5P_DEFAULT};
};

inline
hep_hpc::hdf5::PropertyList::
PropertyList(hid_t const propClassID)
  :
  h5plist_(&H5Pcreate, &H5Pclose, propClassID)
{
  if (*h5plist_ < 0) { // Error that we didn't throw over.
    h5plist_.release();
  }
}

inline
hep_hpc::hdf5::PropertyList::
PropertyList(PropertyList const & other)
  :
  h5plist_(&H5Pcopy, &H5Pclose, *other.h5plist_)
{
  if (*h5plist_ < 0) { // Error that we didn't throw over.
    h5plist_.release();
  }
}

inline
hep_hpc::hdf5::PropertyList &
hep_hpc::hdf5::PropertyList::
operator = (PropertyList const & other)
{
  h5plist_ = { &H5Pcopy, &H5Pclose, *other.h5plist_ };
  if (*h5plist_ < 0) { // Error that we didn't throw over.
    h5plist_.release();
  }
  return *this;
}

inline
bool
hep_hpc::hdf5::PropertyList::
is_valid () const noexcept
{
  return *h5plist_ > INVALID_PLIST_();
}

inline
hep_hpc::hdf5::PropertyList::
operator bool () const noexcept
{
  return is_valid();
}

inline
bool
hep_hpc::hdf5::PropertyList::
is_default() const noexcept
{
  return *h5plist_ == H5P_DEFAULT;
}

inline
bool
hep_hpc::hdf5::PropertyList::
is_valid_non_default() const noexcept
{
  return *h5plist_ > H5P_DEFAULT;
}

inline
hep_hpc::hdf5::PropertyList::
operator hid_t () const noexcept
{
  return *h5plist_;
}

template <typename FUNC, typename... Args>
inline
hep_hpc::hdf5::PropertyList &
hep_hpc::hdf5::PropertyList::
operator () (FUNC func, Args && ... args)
{
  (void) ErrorController::call(ErrorMode::EXCEPTION, func,
                               (*this),
                               std::forward<Args>(args)...);
  return *this;
}

inline
std::string
hep_hpc::hdf5::PropertyList::
getClassName() const {
  // Return from H5Pget_class_name() must be memory-managed.
  hep_hpc::detail::SimpleRAII<char *>
    cname(&H5Pget_class_name, &H5free_memory, ErrorController::call(&H5Pget_class, *h5plist_));
  return *cname;
}

inline
bool
hep_hpc::hdf5::PropertyList::
isClass(hid_t const propClassID) const
{
  return (H5Pequal(ErrorController::call(&H5Pget_class,*h5plist_), propClassID) == 1);
}

inline
void
hep_hpc::hdf5::PropertyList::
reset() {
  using std::swap;
  // Note: we're not calling Resource::reset() because we want the
  // default value.
  PropertyList tmp;
  swap(*this, tmp);
}

#endif /* hep_hpc_hdf5_PropertyList_hpp */
