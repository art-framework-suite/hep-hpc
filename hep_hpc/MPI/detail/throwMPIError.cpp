#include "hep_hpc/MPI/detail/throwMPIError.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

#include "mpi.h"

void
hep_hpc::detail::
throwMPIError(std::string const & funcString, int status)
{
  using namespace std::string_literals;
  using std::to_string;
  char eString[MPI_MAX_ERROR_STRING], eClassString[MPI_MAX_ERROR_STRING];
  int len, eClass;
  (void) MPI_Error_class(status, &eClass);
  (void) MPI_Error_string(eClass, eClassString, &len);
  (void) MPI_Error_string(status, eString, &len);
  throw std::runtime_error("MPI Error: "s +
                           eString + "(" +
                           to_string(status) +
                           ") of class " +
                           eClassString +
                           " while calling " +
                           funcString +
                           ".");
}
