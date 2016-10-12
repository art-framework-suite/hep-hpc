#include "hep_hpc/MPICommunicator.hpp"
#include "hep_hpc/MPIGroup.hpp"

#include "gtest/gtest.h"

#include <iostream>

using namespace hep_hpc;

TEST(MPICommunicator, size)
{
  MPICommunicator const wcomm;
  auto const sz = wcomm.size();
  ASSERT_GT(sz, 0);
}

TEST(MPICommunicator, rank)
{
  MPICommunicator const wcomm;
  auto const rnk = wcomm.rank();
  auto const sz = wcomm.size();
  ASSERT_GT(sz, rnk);
}

TEST(MPICommunicator, split)
{
  MPICommunicator const wcomm, comm(wcomm.split(wcomm.rank() % 2, wcomm.rank()));
  auto const g1 = comm.group();
  auto const g2 = comm.group();
  auto const g1g = static_cast<MPI_Group>(g1);
  auto const g2g = static_cast<MPI_Group>(g2);
  ASSERT_EQ(g1g, g2g);
}

TEST(MPICommunicator, duplicate)
{
  MPICommunicator const wcomm, comm(wcomm.duplicate());
  auto const sz1 = wcomm.size();
  auto const sz2 = comm.size();
  ASSERT_EQ(sz1, sz2);
  auto const g1 = wcomm.group();
  auto const g2 = comm.group();
  auto const g1g = static_cast<MPI_Group>(g1);
  auto const g2g = static_cast<MPI_Group>(g2);
  ASSERT_NE(g1g, g2g);
}

TEST(MPICommunicator, create)
{
  MPICommunicator const wcomm;
  auto const g1 = wcomm.group();
  // TODO: finish this test.  
}
