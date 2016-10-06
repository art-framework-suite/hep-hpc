////////////////////////////////////////////////////////////////////////
// MPI_except_t
//
// Demonstrate safe shutdown of MPI under abnormal conditions (exception
// throw from throwOnMPIError()).
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/MPIInstance.hpp"
#include "hep_hpc/throwOnMPIError.hpp"

#include "mpi.h"

#include <iostream>
#include <stdexcept>

using namespace hep_hpc;

int main(int argc, char **argv)
{
  MPIInstance theMPI(argc, argv, MPIInstance::world_errors_return_tag);
  try {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
      throwOnMPIError(&MPI_Error_string, -1, nullptr, nullptr);
    }
  } catch (std::exception & e) {
    std::cerr << "MPI application caught exception at the top level and will exit:\n"
              << e.what()
              << "\n";
    theMPI.abort(5);
  }
}
