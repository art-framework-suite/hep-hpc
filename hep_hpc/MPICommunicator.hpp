#ifndef HDFSTUDY_MPICOMMUNICATOR_HPP
#define HDFSTUDY_MPICOMMUNICATOR_HPP
////////////////////////////////////////////////////////////////////////
// hep_hpc::MPIComm
//
// Simple (not yet full-featured) class managing an MPI communicator.
//
////////////////////////////////////////////////////////////////////////
// Notes.
//
// * Once an MPI_Comm is used to initialize an MPICommuniator, then it
//   becomes managed by same and should not be freed explicitly.
//
// * MPI_COMM_WORLD, MPI_COMM_NULL and MPI_COMM_SELF are handled
//   specifically, and are correctly not freed upon destruction of an
//   MPICommunicator initialized with same.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/SimpleRAII.hpp"
#include "hep_hpc/throwOnMPIError.hpp"

#include "mpi.h"

namespace hep_hpc {
  class MPICommunicator;

  class MPIGroup;
}

class hep_hpc::MPICommunicator {
public:
  explicit MPICommunicator(MPI_Comm comm = MPI_COMM_WORLD) noexcept;

  // Basic information.
  int size() const;
  int rank() const;

  // Make new communicators.
  MPICommunicator create(MPICommunicator const & communicator,
                         MPIGroup const & group) const;
  MPICommunicator duplicate() const;
  MPICommunicator split(int color, int key) const;

  // Other useful functions.
  MPIGroup group() const;

  // Use with caution: do not call MPI_Comm_free()!
  MPI_Comm comm() const noexcept;

private:
  SimpleRAII<MPI_Comm> theCommunicator_;
};

inline
int
hep_hpc::MPICommunicator::
size() const
{
  int result;
  throwOnMPIError("MPI_Comm_size", &MPI_Comm_size, *theCommunicator_, &result);
  return result;
}

inline
int
hep_hpc::MPICommunicator::
rank() const
{
  int result;
  throwOnMPIError("MPI_Comm_rank", &MPI_Comm_rank, *theCommunicator_, &result);
  return result;
}

inline
auto
hep_hpc::MPICommunicator::
duplicate() const
->MPICommunicator
{
  MPI_Comm c;
  throwOnMPIError("MPI_Comm_dup", &MPI_Comm_dup,
                  *theCommunicator_, &c);
  return MPICommunicator(c);
}

inline
auto
hep_hpc::MPICommunicator::
split(int color, int key) const
->MPICommunicator
{
  MPI_Comm c;
  throwOnMPIError("MPI_Comm_split", &MPI_Comm_split,
                  *theCommunicator_, color, key, &c);
  return MPICommunicator(c);
}

inline
MPI_Comm
hep_hpc::MPICommunicator::
comm() const noexcept
{
  return *theCommunicator_;
}
#endif /* HDFSTUDY_MPICOMMUNICATOR_HPP */
