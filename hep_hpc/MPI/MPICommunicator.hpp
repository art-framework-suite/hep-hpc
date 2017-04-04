#ifndef hep_hpc_MPI_MPICommunicator_hpp
#define hep_hpc_MPI_MPICommunicator_hpp
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
#include "hep_hpc/MPI/MPIComparison_t.hpp"
#include "hep_hpc/Utilities/SimpleRAII.hpp"
#include "hep_hpc/MPI/throwOnMPIError.hpp"

#include "mpi.h"

namespace hep_hpc {
  class MPICommunicator;

  class MPIGroup;

  // Comparison operator.
  bool operator == (MPICommunicator const & left,
                    MPICommunicator const & right);
  bool operator != (MPICommunicator const & left,
                    MPICommunicator const & right);
}

class hep_hpc::MPICommunicator {
public:
  explicit MPICommunicator(MPI_Comm comm = MPI_COMM_WORLD) noexcept;

  // Basic information.
  int size() const;
  int rank() const;

  // Compare with another communicator (answers are symmetric).
  MPIComparison_t compare(MPICommunicator const & other) const;

  // Make new communicators.
  MPICommunicator create(MPICommunicator const & communicator,
                         MPIGroup const & group) const;
  MPICommunicator duplicate() const;
  MPICommunicator split(int color, int key) const;

  // Obtain the group corresponding to this communicator.
  MPIGroup group() const;

  // Provide acccess to the underlying communicator while retaining
  // resource control.
  operator MPI_Comm() const noexcept;

private:
  detail::SimpleRAII<MPI_Comm> theCommunicator_;
};

inline
int
hep_hpc::MPICommunicator::
size() const
{
  int result;
  throwOnMPIError("MPI_Comm_size()", &MPI_Comm_size, *theCommunicator_, &result);
  return result;
}

inline
auto
hep_hpc::MPICommunicator::
compare(MPICommunicator const & other) const
-> MPIComparison_t
{
  int result;
  throwOnMPIError("MPI_Comm_compare()", &MPI_Comm_compare, *theCommunicator_, other, &result);
  return MPIComparison_t(result);
}

inline
int
hep_hpc::MPICommunicator::
rank() const
{
  int result;
  throwOnMPIError("MPI_Comm_rank()", &MPI_Comm_rank, *theCommunicator_, &result);
  return result;
}

inline
auto
hep_hpc::MPICommunicator::
duplicate() const
->MPICommunicator
{
  MPI_Comm c;
  throwOnMPIError("MPI_Comm_dup()", &MPI_Comm_dup, *theCommunicator_, &c);
  return MPICommunicator(c);
}

inline
auto
hep_hpc::MPICommunicator::
split(int color, int key) const
->MPICommunicator
{
  MPI_Comm c;
  throwOnMPIError("MPI_Comm_split()", &MPI_Comm_split, *theCommunicator_, color, key, &c);
  return MPICommunicator(c);
}

inline
hep_hpc::MPICommunicator::
operator
MPI_Comm() const noexcept
{
  return *theCommunicator_;
}

inline
bool
hep_hpc::operator == (MPICommunicator const & left,
                       MPICommunicator const & right)
{
  return left.compare(right) == MPIComparison_t::IDENTICAL;
}

inline
bool
hep_hpc::
operator != (MPICommunicator const & left,
             MPICommunicator const & right)
{
  return !(left == right);
}
#endif /* hep_hpc_MPI_MPICommunicator_hpp */
