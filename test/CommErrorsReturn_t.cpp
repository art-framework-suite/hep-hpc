////////////////////////////////////////////////////////////////////////
// MPI_except_t
//
// Demonstrate inheritance of error handlers to new communicators.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/MPIInstance.hpp"
#include "hep_hpc/throwOnMPIError.hpp"

#include "mpi.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace hep_hpc;

int main(int argc, char **argv)
{
  using namespace std::string_literals;
  using std::to_string;
  MPIInstance theMPI(argc, argv, MPIInstance::world_errors_return_tag);
  try {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm splitComm;
    MPI_Comm_split(MPI_COMM_WORLD, rank % 2, rank, &splitComm);
    MPI_Errhandler worldHandler, splitHandler;
    MPI_Errhandler_get(MPI_COMM_WORLD, &worldHandler);
    MPI_Errhandler_get(splitComm, &splitHandler);
    if (rank == 0) {
      if (worldHandler != splitHandler) {
        throw std::runtime_error("ERROR: Error handler comparison: "s +
                                 to_string(worldHandler) +
                                 " != " +
                                 to_string(splitHandler));
      }
      MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_ARE_FATAL);
      throwOnMPIError(&MPI_Comm_call_errhandler,
                      splitComm,
                      MPI_ERR_OP);
    }
  } catch (std::exception & e) {
    std::cerr << "MPI application caught exception at the top level and will exit:\n"
              << e.what()
              << "\n";
    theMPI.abort(5);
  }
}
