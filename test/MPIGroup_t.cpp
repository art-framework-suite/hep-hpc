#include "hep_hpc/MPICommunicator.hpp"
#include "hep_hpc/MPIGroup.hpp"

#include "gtest/gtest.h"

using namespace hep_hpc;

TEST(Group, size) 
{
  MPIGroup const empty, group(MPICommunicator().group());
  auto const sz1 = empty.size();
  ASSERT_EQ(sz1, 0);
  auto const sz2 = group.size();
  ASSERT_GT(sz2, sz1);
}

TEST(Group, rank) 
{
  MPICommunicator const wcomm;
  MPIGroup const empty, group(wcomm.group());
  auto const rank1 = empty.rank();
  ASSERT_TRUE(rank1 == MPI_UNDEFINED)
    << "Rank of empty group should be undefined!";
  auto const rank2 = wcomm.rank();
  ASSERT_TRUE(rank2 == wcomm.rank())
    << "Rank of world group should be rank of world communicator!";
}

TEST(Group, compare)
{
  MPICommunicator const wcomm;  
  MPIGroup const empty, group(wcomm.group());
  ASSERT_TRUE(empty.compare(empty) == MPIComparison_t::IDENTICAL);
  ASSERT_TRUE(group.compare(group) == MPIComparison_t::IDENTICAL);
  ASSERT_TRUE(group.compare(empty) == MPIComparison_t::UNEQUAL);
  ASSERT_TRUE(empty == empty);
  ASSERT_TRUE(group == group);
  ASSERT_FALSE(empty == group);
  ASSERT_FALSE(empty != empty);
  ASSERT_FALSE(group != group);
  ASSERT_TRUE(empty != group);
}

TEST(Group, difference)
{
  MPICommunicator const wcomm, scomm(MPI_COMM_SELF);
  MPIGroup const wg = wcomm.group(), sg = scomm.group(), eg;
  ASSERT_TRUE(wg.difference(sg).rank() == MPI_UNDEFINED);
  ASSERT_TRUE(wg.difference(eg).compare(wg) == MPIComparison_t::IDENTICAL);
}
