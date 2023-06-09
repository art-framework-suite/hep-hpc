////////////////////////////////////////////////////////////////////////
// MPIInstance_t
//
// Demonstrate safe shutdown of MPI under normal conditions.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/MPI/MPIInstance.hpp"

#include <stdexcept>

using namespace hep_hpc;

int main(int argc, char **argv)
{
  MPIInstance theMPI(argc, argv);
}
