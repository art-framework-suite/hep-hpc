#include "hep_hpc/MPIInstance.hpp"
#include "hep_hpc/throwOnMPIError.hpp"

#include "mpi.h"

#include <iostream>

hep_hpc::MPIInstance::MPIInstance(int & argc, char **& argv)
{
  int provided;
  auto const required = MPI_THREAD_FUNNELED;
  throwOnMPIError("MPI_Init_thread",
                  &MPI_Init_thread, &argc, &argv, required, &provided);
  if (provided != required) {
    std::cerr << "hep_hpc::MPIInstance::MPIInstance() unable to obtain requested thread operation level ("
              << required << "): received thread level " << provided << ".\n";
    this->abort();
  }
}

hep_hpc::MPIInstance::MPIInstance(int & argc, char **& argv, WORLD_ERRORS_RETURN_t)
  :
  MPIInstance(argc, argv)
{
  MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);  
}

hep_hpc::MPIInstance::~MPIInstance() {
  MPI_Finalize();
}

void
hep_hpc::MPIInstance::abort(int exit_code) {
  MPI_Abort(MPI_COMM_WORLD, exit_code);
}
