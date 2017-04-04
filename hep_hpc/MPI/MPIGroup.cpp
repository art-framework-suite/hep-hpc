#include "hep_hpc/MPI/MPIGroup.hpp"

#include "hep_hpc/MPI/MPICommunicator.hpp"

namespace {
  auto teardown =
    [](MPI_Group group) noexcept
  {
    if (group != MPI_GROUP_EMPTY &&
        group != MPI_GROUP_NULL) {
      MPI_Group_free(&group);
    }
  };
}

hep_hpc::MPIGroup::
MPIGroup(MPI_Group group) noexcept
  :
theGroup_([](MPI_Group group) noexcept { return group; }, teardown, group)
{
}
