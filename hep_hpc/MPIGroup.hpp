#ifndef HDFSTUDY_MPIGROUP_HPP
#define HDFSTUDY_MPIGROUP_HPP
////////////////////////////////////////////////////////////////////////
// hep_hpc::MPIGroup
//
// Simple (not yet full-featured class managing an MPI group.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/SimpleRAII.hpp"

#include "hep_hpc/throwOnMPIError.hpp"

#include "mpi.h"

namespace hep_hpc {
  class MPIGroup;

  // Forward declaration.
  class MPICommunicator;
}

class hep_hpc::MPIGroup {
public:
  explicit MPIGroup(MPI_Group group = MPI_GROUP_EMPTY) noexcept;

  // Basic information.
  int size() const;
  int rank() const;

  // Use with caution: do not call MPI_Group_free()!
  MPI_Group group() const noexcept;
  
private:
  SimpleRAII<MPI_Group> theGroup_;
};

inline
int
hep_hpc::MPIGroup::
size() const
{
  int result;
  throwOnMPIError(&MPI_Group_size, *theGroup_, &result);
  return result;
}

inline
int
hep_hpc::MPIGroup::
rank() const
{
  int result;
  throwOnMPIError(&MPI_Group_rank, *theGroup_, &result);
  return result;
}

inline
MPI_Group
hep_hpc::MPIGroup::
group() const noexcept
{
  return *theGroup_;
}

#endif /* HDFSTUDY_MPIGROUP_HPP */
