#include "hep_hpc/hdf5/Group.hpp"
#include "hep_hpc/hdf5/Exception.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

hep_hpc::hdf5::Group::
Group(hid_t const fileOrGroup,
      std::string const & fullPathName,
      group_mode_t const mode,
      PropertyList linkCreationProperties,
      PropertyList groupCreationProperties,
      PropertyList groupAccessProperties)
  :
  h5group_([&]()
           { HID_t result;
             switch (mode) {
             case CREATE_MODE:
               result =
                 ErrorController::call(&H5Gcreate2,
                                       fileOrGroup,
                                       fullPathName.c_str(),
                                       linkCreationProperties,
                                       groupCreationProperties,
                                       groupAccessProperties);
               break;
             case OPEN_MODE:
               if (linkCreationProperties || groupCreationProperties) { // ERROR.
                 throw
                   Exception("Group(): flags indicated file-open semantics, "
                             "but non-trivial creation properties specified!");
               }
               result =
                 ErrorController::call(&H5Gopen2,
                                       fileOrGroup,
                                       fullPathName.c_str(),
                                       groupAccessProperties);
               break;
             case OPEN_OR_CREATE_MODE:
               // Attempt to open, no error handling.
               result = ErrorController::call(ErrorMode::NONE,
                                              &H5Gopen2,
                                              fileOrGroup,
                                              fullPathName.c_str(),
                                              groupAccessProperties);
               if (result < 0) {
                 // Attempt to create, normal error handling.
                 result =
                   ErrorController::call(&H5Gcreate,
                                         fileOrGroup,
                                         fullPathName.c_str(),
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

hep_hpc::hdf5::HID_t const
hep_hpc::hdf5::Group::INVALID_GROUP_;
