#include "hep_hpc/ScopedErrorHandler.hpp"

#include "hep_hpc/errorHandling.hpp"

hep_hpc::ScopedErrorHandler::ScopedErrorHandler()
  :
  ScopedErrorHandler(NULL, NULL)
{
}

hep_hpc::ScopedErrorHandler::
ScopedErrorHandler(H5E_auto2_t func, void * clientData)
  :
  errHandler_(&setAndSaveErrorHandler, &restoreErrorHandler, func, clientData)
{
}
