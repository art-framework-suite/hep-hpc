#ifndef hep_hpc_MPI_MPIGroup_hpp
#define hep_hpc_MPI_MPIGroup_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::MPIGroup
//
// Class managing an MPI group.
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/MPI/MPIComparison_t.hpp"
#include "hep_hpc/Utilities/SimpleRAII.hpp"
#include "hep_hpc/MPI/throwOnMPIError.hpp"

#include "mpi.h"

#include <iterator>
#include <vector>

namespace hep_hpc {
  class MPIGroup;

  // Comparison operators.
  bool operator == (MPIGroup const & left,
                    MPIGroup const & right);
  bool operator != (MPIGroup const & left,
                    MPIGroup const & right);
}

class hep_hpc::MPIGroup {
public:
  explicit MPIGroup(MPI_Group group = MPI_GROUP_EMPTY) noexcept;

  // Basic information.
  int rank() const;
  int size() const;

  // Compare with another group (answers are symmetric).
  MPIComparison_t compare(MPIGroup const & other) const;

  // Return a new group whose members are those in this group missing
  // from the comparison group.
  MPIGroup difference(MPIGroup const & other) const;

  // Return a new group whose members are those in this group mentioned
  // in the specified rank or ranks.
  template <typename ITER>
  MPIGroup incl(ITER begin, ITER end) const;
  MPIGroup incl(std::size_t sz, int const arr[]) const;
  MPIGroup incl(int const * begin, int const * end) const;
  MPIGroup incl(int inc_rank) const;

  // Return a new group whose members are those in this group *not*
  // mentioned in the specified rank or ranks.
  template <typename ITER>
  MPIGroup excl(ITER begin, ITER end) const;
  MPIGroup excl(std::size_t sz, int const arr[]) const;
  MPIGroup excl(int const * begin, int const * end) const;
  MPIGroup excl(int ex_rank) const;

  // Return a new group whose members are the intersection of those in
  // this group with another.
  MPIGroup intersection(MPIGroup const & other) const;

  // Return a new group whose members are the union of those in
  // this group with another.
  MPIGroup join(MPIGroup const & other) const;

  // Provide access to the underlying group while retaining resource
  // control.
  operator MPI_Group() const noexcept;
  
private:
  detail::SimpleRAII<MPI_Group> theGroup_;
};

inline
int
hep_hpc::MPIGroup::
rank() const
{
  int result;
  throwOnMPIError("MPI_Group_rank()", &MPI_Group_rank, *theGroup_, &result);
  return result;
}

inline
int
hep_hpc::MPIGroup::
size() const
{
  int result;
  throwOnMPIError("MPI_Group_size()", &MPI_Group_size, *theGroup_, &result);
  return result;
}

inline
auto
hep_hpc::MPIGroup::
compare(MPIGroup const & other) const
-> MPIComparison_t
{
  int result;
  throwOnMPIError("MPI_Group_compare()", &MPI_Group_compare, *theGroup_, other, &result);
  return MPIComparison_t(result);
}

inline
auto
hep_hpc::MPIGroup::
difference(MPIGroup const & other) const
-> MPIGroup
{
  MPI_Group g;
  throwOnMPIError("MPI_Group_difference()", &MPI_Group_difference, *theGroup_, other, &g);
  return MPIGroup(g);
}

template <typename ITER>
inline
auto
hep_hpc::MPIGroup::
incl(ITER begin, ITER end) const
-> MPIGroup
{
  MPI_Group g;
  std::vector<int> const v(begin, end);
  throwOnMPIError("MPI_Group_incl()", &MPI_Group_incl, *theGroup_, v.size(), v.data(), &g);
  return MPIGroup(g);
}

inline
auto
hep_hpc::MPIGroup::
incl(std::size_t const sz, int const arr[]) const
-> MPIGroup
{
  MPI_Group g;
  throwOnMPIError("MPI_Group_incl()", &MPI_Group_incl, *theGroup_, sz, arr, &g);
  return MPIGroup(g);
}

inline
auto
hep_hpc::MPIGroup::
incl(int const * begin,
     int const * end) const
-> MPIGroup
{
  return incl(end - begin, begin);
}

inline
auto
hep_hpc::MPIGroup::
incl(int const inc_rank) const
-> MPIGroup
{
  return incl(1ul, &inc_rank);
}

template <typename ITER>
inline
auto
hep_hpc::MPIGroup::
excl(ITER begin, ITER end) const
-> MPIGroup
{
  MPI_Group g;
  std::vector<int> const v(begin, end);
  throwOnMPIError("MPI_Group_excl()", &MPI_Group_excl, *theGroup_, v.size(), v.data(), &g);
  return MPIGroup(g);
}

inline
auto
hep_hpc::MPIGroup::
excl(std::size_t const sz, int const arr[]) const
-> MPIGroup
{
  MPI_Group g;
  throwOnMPIError("MPI_Group_excl()", &MPI_Group_excl, *theGroup_, sz, arr, &g);
  return MPIGroup(g);
}

inline
auto
hep_hpc::MPIGroup::
excl(int const * begin,
     int const * end) const
-> MPIGroup
{
  return excl(end - begin, begin);
}

inline
auto
hep_hpc::MPIGroup::
excl(int const inc_rank) const
-> MPIGroup
{
  return excl(1ul, &inc_rank);
}

inline
auto
hep_hpc::MPIGroup::
intersection(MPIGroup const & other) const
-> MPIGroup
{
  MPI_Group g;
  throwOnMPIError("MPI_Group_intersection()", &MPI_Group_intersection, *theGroup_, other, &g);
  return MPIGroup(g);
}

inline
auto
hep_hpc::MPIGroup::
join(MPIGroup const & other) const
-> MPIGroup
{
  MPI_Group g;
  throwOnMPIError("MPI_Group_union()", &MPI_Group_union, *theGroup_, other, &g);
  return MPIGroup(g);
}

inline
hep_hpc::MPIGroup::
operator MPI_Group() const noexcept
{
  return *theGroup_;
}

inline
bool
hep_hpc::
operator == (MPIGroup const & left, MPIGroup const & right)
{
  return left.compare(right) == MPIComparison_t::IDENTICAL;
}

inline
bool
hep_hpc::
operator != (MPIGroup const & left, MPIGroup const & right)
{
  return !(left == right);
}
#endif /* hep_hpc_MPI_MPIGroup_hpp */
