#include "hep_hpc/MPI.hpp"

#include "mpi.h"

#include <iostream>

hep_hpc::MPI::MPI(int & argc, char **& argv)
{
  int provided;
  auto const required = MPI_THREAD_FUNNELED;
  (void) MPI_Init_thread(&argc, &argv, required, &provided);
  if (provided != required) {
    std::cerr << "hep_hpc::MPI::MPI() unable to obtain requested thread operation level ("
              << required << "): received thread level " << provided << ".\n";
    this->abort();
  }
}

hep_hpc::MPI::~MPI() {
  MPI_Finalize();
}

void
hep_hpc::MPI::abort(int exit_code) {
  MPI_Abort(MPI_COMM_WORLD, exit_code);
}
