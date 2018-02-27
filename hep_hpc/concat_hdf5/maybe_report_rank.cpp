#include "hep_hpc/concat_hdf5/maybe_report_rank.hpp"

#include "hep_hpc/detail/config.hpp"
#ifdef HEP_HPC_USE_MPI
#include "hep_hpc/MPI/MPICommunicator.hpp"
using namespace hep_hpc;
#endif

#include <iomanip>
#include <ostream>
#include <sstream>

namespace {
#ifdef HEP_HPC_USE_MPI
  std::string compose_msg()
  {
    std::string result;
    MPICommunicator world;
    std::ostringstream msg;
    auto const wsz = world.size();
    auto const width_wsz = std::to_string(wsz).size();
    msg << '['
        << std::setw(width_wsz)
        << world.rank()
        << '/'
        << world.size()
        << "] ";
    result = msg.str();
    return result;
  }
#endif
}

std::ostream & maybe_report_rank(std::ostream & os)
{
#ifdef HEP_HPC_USE_MPI
  static std::string const msg(compose_msg());
  os << msg;
#endif
  return os;
}
