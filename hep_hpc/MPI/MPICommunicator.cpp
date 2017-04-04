#include "hep_hpc/MPI/MPICommunicator.hpp"
#include "hep_hpc/MPI/MPIGroup.hpp"

namespace {
  auto teardown =
    [](MPI_Comm comm) noexcept
  {
    if (comm != MPI_COMM_WORLD &&
        comm != MPI_COMM_SELF &&
        comm != MPI_COMM_NULL) {
      MPI_Comm_free(&comm);
    }
  };
}

hep_hpc::MPICommunicator::
MPICommunicator(MPI_Comm comm) noexcept 
  :
  theCommunicator_([](MPI_Comm comm) noexcept { return comm; }, teardown, comm)
{
}

auto
hep_hpc::MPICommunicator::
create(MPICommunicator const & communicator,
       MPIGroup const & group) const
-> MPICommunicator
{
  MPI_Comm c;
  throwOnMPIError("MPI_Comm_create()", &MPI_Comm_create,
                          communicator,
                          group,
                          &c);
  return MPICommunicator(c);
}

auto
hep_hpc::MPICommunicator::
group() const
-> MPIGroup
{
  MPI_Group g;
  throwOnMPIError("MPI_Comm_group()", &MPI_Comm_group, *theCommunicator_, &g);
  return MPIGroup(g);
}
