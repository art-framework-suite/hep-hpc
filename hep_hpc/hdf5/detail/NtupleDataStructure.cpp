#include "hep_hpc/hdf5/detail/NtupleDataStructure.hpp"

hep_hpc::hdf5::Group
hep_hpc::hdf5::detail::
makeGroup(hid_t file, std::string const & name, bool overwriteContents)
{
  using namespace std::string_literals;
  Group group;
  {
    ScopedErrorHandler seh;
    group = Group(file, name);
  }
  if (!group) { // Already exists.
    if (!overwriteContents) {
      throw std::runtime_error("Group "s + name +
                               " already exists and overwriting is not specified."s);
    }
    ErrorController::call(ErrorMode::EXCEPTION,
                                "Group "s + name + " cannot be deleted!"s,
                                &H5Ldelete, file, name.c_str(), H5P_DEFAULT);
    group = Group(file, name);
  }
  return group;
}
               
