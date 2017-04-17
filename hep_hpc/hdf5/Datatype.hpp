#ifndef KKKFYUFYRYYAGHGS
#define KKKFYUFYRYYAGHGS
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

  // Take ownership.
  explicit Datatype(HID_t dtype);

  // Copy operations.
  Datatype(Datatype const & other);
  Datatype & operator = (Datatype const & other);

  // Default move operations.
  Datatype(Datatype &&) = default;
  Datatype & operator = (Datatype &&) = default;

  // Is this a valid, non-default dataspace?
  explicit operator bool () const noexcept;

  // Access to the underlying resource handle.
  operator hid_t() const noexcept;

  // Explicitly reset (not usually necessary).
  void reset();

private:
  static HID_t const INVALID_DTYPE_;
  Resource<HID_t> h5dtype_;
};

inline
hep_hpc::hdf5::Datatype::
Datatype(HID_t const dtype)
  :
  h5dtype_(dtype, &H5Tclose)
{
}

inline
hep_hpc::hdf5::Datatype::
Datatype(Datatype const & other)
  :
  h5dtype_(&H5Tcopy, &H5Tclose, *other.h5dtype_)
{
  std::cerr << "Copying a dtype.\n";
}

inline
hep_hpc::hdf5::Datatype &
hep_hpc::hdf5::Datatype::
operator = (Datatype const & other)
{
  h5dtype_ = {&H5Tcopy, &H5Tclose, *other.h5dtype_};
  return *this;
}

inline
hep_hpc::hdf5::Datatype::
operator bool () const noexcept
{
  return *h5dtype_ > INVALID_DTYPE_;
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

#endif
