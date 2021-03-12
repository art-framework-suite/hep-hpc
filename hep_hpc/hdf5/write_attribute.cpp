#include "hep_hpc/hdf5/write_attribute.hpp"
#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/Resource.hpp"
#include "hep_hpc/hdf5/Datatype.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

herr_t
hep_hpc::hdf5::
write_attribute(hid_t groupOrDataset,
                std::string const & name,
                std::string const & value)
{
  herr_t result = -1;

  // Create the string type.
  Datatype string_type(ErrorController::call(&H5Tcopy, H5T_C_S1));

  if (!string_type.is_valid()) {
    // If we're not using the recommended exception-based error
    // handling.
    result = string_type;
    return result;
  }

  result = ErrorController::call(&H5Tset_size,
                                 string_type,
                                 value.size() + 1);
  if (result < 0) {
    // If we're not using the recommended exception-based error
    // handling.
    return result;
  }

  // Create a simple dataspace.
  hsize_t const extent = 1;
  Dataspace dspace(1, &extent);

  if (!dspace.is_valid()) {
    // If we're not using the recommended exception-based error
    // handling.
    result = dspace;
    return result;
  }

  Resource a_id(ErrorController::call(&H5Acreate,
                                      groupOrDataset,
                                      name.c_str(),
                                      string_type,
                                      dspace,
                                      H5P_DEFAULT, H5P_DEFAULT),
                &H5Aclose);

  if (*a_id < 0) {
    // If we're not using the recommended exception-based error
    // handling.
    result = *a_id;
    return result;
  }

  result = ErrorController::call(&H5Awrite,
                                 *a_id,
                                 string_type,
                                 (void *) value.c_str());
  return result;
}

herr_t
hep_hpc::hdf5::
write_attribute(hid_t groupOrDataset,
		std::string const & name,
		std::vector<std::string> const & values)
{
  herr_t result = -1;

  // Create the string type.
  Datatype string_type(ErrorController::call(&H5Tcopy, H5T_C_S1));
  
  if (!string_type.is_valid()) {
    // If we're not using the recommended exception-based error
    // handling.
    result = string_type;
    return result;
  }

  result = ErrorController::call(&H5Tset_size,
                                 string_type,
                                 H5T_VARIABLE);
  if (result < 0) {
    // If we're not using the recommended exception-based error
    // handling.
    return result;
  }

  // Create a simple dataspace.
  hsize_t const extent = values.size();
  Dataspace dspace(1, &extent);

  if (!dspace.is_valid()) {
    // If we're not using the recommended exception-based error
    // handling.
    result = dspace;
    return result;
  }

  Resource a_id(ErrorController::call(&H5Acreate,
                                      groupOrDataset,
                                      name.c_str(),
                                      string_type,
                                      dspace,
                                      H5P_DEFAULT, H5P_DEFAULT),
                &H5Aclose);

  if (*a_id < 0) {
    // If we're not using the recommended exception-based error
    // handling.
    result = *a_id;
    return result;
  }


  // convert vector of strings into C string array
  std::vector<const char*> c_values;
  for(auto i = 0u; i < values.size(); i++)
    c_values.push_back(values[i].c_str());

  
  result = ErrorController::call(&H5Awrite,
                                 *a_id,
                                 string_type,
                                 (void *) &c_values[0]);
  return result;
}
