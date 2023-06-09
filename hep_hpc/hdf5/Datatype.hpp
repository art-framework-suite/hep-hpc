#ifndef hep_hpc_hdf5_Datatype_hpp
#define hep_hpc_hdf5_Datatype_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::Datatype
//
// Class representing an HDF5 type, with resource management.
//
// N.B. Datatypes do not generally need to last beyond the call
//      passing them to an HDF5 function.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/hdf5/Resource.hpp"
#include "hep_hpc/hdf5/ResourceStrategy.hpp"

#include "hdf5.h"

#include <iostream>

namespace hep_hpc {
  namespace hdf5 {
    class Datatype;
  }
}

class hep_hpc::hdf5::Datatype {
public:
  Datatype() = default;

  // Adopt an existing HDF5 datatype ID with the specified management
  // strategy. If observing, caller is responsible for closing at the
  // appropriate time.
  explicit Datatype(HID_t dtype,
                    ResourceStrategy strategy = ResourceStrategy::handle_tag);

  // Copy operations.
  Datatype(Datatype const & other);
  Datatype & operator = (Datatype const & other);

  // Default move operations.
  Datatype(Datatype &&) = default;
  Datatype & operator = (Datatype &&) = default;

  // Is this a valid datatype?
  bool is_valid() const noexcept;
  explicit operator bool () const noexcept;

  // Access to the underlying resource handle.
  operator hid_t() const noexcept;

  // Explicitly reset (not usually necessary).
  void reset();

private:
  static constexpr HID_t INVALID_DTYPE_() { return HID_t {};}
  Resource h5dtype_;
};

inline
hep_hpc::hdf5::Datatype::
Datatype(HID_t const dtype,
         ResourceStrategy const strategy)
  :
  h5dtype_((strategy == ResourceStrategy::handle_tag) ?
           Resource(dtype, &H5Tclose) :
           Resource(dtype))
{
}

inline
hep_hpc::hdf5::Datatype::
Datatype(Datatype const & other)
  :
  h5dtype_(&H5Tcopy, &H5Tclose, *other.h5dtype_)
{
  if (*h5dtype_ < 0) { // Error that we didn't throw over.
    h5dtype_.release();
  }
}

inline
hep_hpc::hdf5::Datatype &
hep_hpc::hdf5::Datatype::
operator = (Datatype const & other)
{
  h5dtype_ = {&H5Tcopy, &H5Tclose, *other.h5dtype_};
  if (*h5dtype_ < 0) { // Error that we didn't throw over.
    h5dtype_.release();
  }
  return *this;
}

inline
bool
hep_hpc::hdf5::Datatype::
is_valid() const noexcept
{
  return *h5dtype_ > INVALID_DTYPE_();
}

inline
hep_hpc::hdf5::Datatype::
operator bool () const noexcept
{
  return is_valid();
}

inline
hep_hpc::hdf5::Datatype::
operator hid_t() const noexcept
{
  return *h5dtype_;
}

inline
void
hep_hpc::hdf5::Datatype::
reset()
{
  h5dtype_.reset();
}

#endif /* hep_hpc_hdf5_Datatype_hpp */
