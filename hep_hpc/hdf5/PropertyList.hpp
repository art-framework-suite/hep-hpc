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

  // Is this a valid, non-default property list?
  explicit operator bool () const noexcept;

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
  // Note we are using a plain hid_t here rather than HID_t, because 0
  // (H5P_DEFAULT) is a reasonable default;
  Resource h5plist_ {0};
};

inline
hep_hpc::hdf5::PropertyList::
PropertyList(hid_t const plist, ResourceStrategy const strategy)
  : h5plist_((strategy == ResourceStrategy::handle_tag) ?
             Resource(plist, &H5Pclose) :
             Resource(plist))
{
}

inline
hep_hpc::hdf5::PropertyList::
PropertyList(hid_t const propClassID)
  :
  h5plist_(&H5Pcreate, &H5Pclose, propClassID)
{
}

inline
hep_hpc::hdf5::PropertyList::
PropertyList(PropertyList const & other)
  :
  h5plist_(&H5Pcopy, &H5Pclose, *other.h5plist_)
{
}

inline
hep_hpc::hdf5::PropertyList &
hep_hpc::hdf5::PropertyList::
operator = (PropertyList const & other)
{
  h5plist_ = { &H5Pcopy, &H5Pclose, *other.h5plist_ };
  return *this;
}

inline
hep_hpc::hdf5::PropertyList::
operator bool () const noexcept
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
  return H5Pequal(ErrorController::call(&H5Pget_class,*h5plist_), propClassID);
}

inline
void
hep_hpc::hdf5::PropertyList::
reset() {
  h5plist_.reset();
}

#endif /* hep_hpc_hdf5_PropertyList_hpp */
