////////////////////////////////////////////////////////////////////////
// MPI_except_t
//
// Demonstrate safe shutdown of MPI under abnormal conditions (exception
// throw).
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/MPI.hpp"

#include "mpi.h"

#include <iostream>
#include <stdexcept>

using namespace hep_hpc;

int main(int argc, char **argv)
{
  hep_hpc::MPI theMPI(argc, argv);
  try {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
      throw std::runtime_error("Initiating exceptional exit.");
    }
  } catch (std::exception & e) {
    std::cerr << "MPI application caught exception at the top level and will exit:\n"
              << e.what()
              << "\n";
    theMPI.abort(5);
  }
}
