#include "hep_hpc/H5Group.hpp"
#include "hep_hpc/H5Exception.hpp"
#include "hep_hpc/ScopedErrorHandler.hpp"

hep_hpc::H5Group::
H5Group(hid_t const fileOrGroup,
        std::string const & fullPathName,
        group_mode_t const mode,
        H5PropertyList const linkCreationProperties,
        H5PropertyList const groupCreationProperties,
        H5PropertyList const groupAccessProperties)
  :
  h5group_([&]()
           { HID_t result;
           switch (mode) {
           case CREATE_MODE:
             result = H5Gcreate2(fileOrGroup, fullPathName.c_str(),
                                 linkCreationProperties,
                                 groupCreationProperties,
                                 groupAccessProperties);
             break;
           case OPEN_MODE:
             if (linkCreationProperties || groupCreationProperties) { // ERROR.
               throw
                 H5Exception("H5Group(): flags indicated file-open semantics, "
                             "but non-trivial creation properties specified!");
             }
             result = H5Gopen2(fileOrGroup, fullPathName.c_str(), groupAccessProperties);
             break;
           case OPEN_OR_CREATE_MODE:
           {
             ScopedErrorHandler seh;
             // Attempt to open.
             result = H5Gopen2(fileOrGroup, fullPathName.c_str(), groupAccessProperties);
           }
           if (result < 0) {
             // Attempt to create.
             result = H5Gcreate(fileOrGroup, fullPathName.c_str(),
                                linkCreationProperties,
                                groupCreationProperties,
                                groupAccessProperties);
           }
           break;
           }
           return result;
         }, &H5Gclose)
{
}

hep_hpc::HID_t const
hep_hpc::H5Group::INVALID_GROUP_;
