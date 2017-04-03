#include "hep_hpc/ScopedErrorHandler.hpp"

hep_hpc::ScopedErrorHandler::ScopedErrorHandler()
  :
  ScopedErrorHandler(NULL, NULL)
{
}

hep_hpc::ScopedErrorHandler::
ScopedErrorHandler(H5E_auto2_t func, void * clientData)
  :
  errHandler_(static_cast<herr_t(*)(H5E_auto2_t, void *)>(&setAndSaveErrorHandler),
              &restoreErrorHandler, func, clientData)
{
}

hep_hpc::ScopedErrorHandler::
ScopedErrorHandler(ErrorMode mode)
  :
  errHandler_(static_cast<herr_t(*)(ErrorMode)>(&setAndSaveErrorHandler), &restoreErrorHandler, mode)
{
}
