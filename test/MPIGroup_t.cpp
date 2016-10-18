#include "hep_hpc/MPICommunicator.hpp"
#include "hep_hpc/MPIGroup.hpp"

#include "gtest/gtest.h"

#include <iterator>
#include <vector>

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

namespace {
  struct GroupFixture : public ::testing::Test {
    GroupFixture();

    MPICommunicator const wcomm;
    MPIGroup const wg;
    std::vector<int> gMembers;

  private:
    std::vector<int> initGMembers() const;
  };

  inline
  GroupFixture::GroupFixture()
    :
    wcomm(),
    wg(wcomm.group()),
    gMembers(initGMembers())
  {
  }

  std::vector<int>
  GroupFixture::initGMembers() const
  {
    std::vector<int> result;
    auto const nRanks = wg.size();
    result.reserve(nRanks / 2);
    for (int i = 0; i < nRanks; i += 2) {
      result.push_back(i);
    }
    return result;
  }
}

TEST_F(GroupFixture, incl)
{
  auto const g1 = wg.incl(gMembers.cbegin(), gMembers.cend()),
             g2 = wg.incl(gMembers.size(), gMembers.data()),
             g3 = wg.incl(gMembers.data(), gMembers.data() + gMembers.size()),
             g4 = wg.incl(wcomm.rank());
  ASSERT_TRUE(static_cast<std::size_t>(g1.size()) == gMembers.size());
  ASSERT_TRUE(g1.rank() == ((wcomm.rank() % 2) ? MPI_UNDEFINED : (wcomm.rank() / 2)));
  ASSERT_EQ(g1, g2);
  ASSERT_EQ(g2, g3);
  ASSERT_TRUE(g4.size() == 1);
  ASSERT_FALSE(g4.rank() == MPI_UNDEFINED);
}

TEST_F(GroupFixture, excl)
{
  auto const g1 = wg.excl(gMembers.cbegin(), gMembers.cend()),
             g2 = wg.excl(gMembers.size(), gMembers.data()),
             g3 = wg.excl(gMembers.data(), gMembers.data() + gMembers.size()),
             g4 = wg.excl(wcomm.rank());
  ASSERT_TRUE(static_cast<std::size_t>(g1.size()) == wg.size() - gMembers.size());
  ASSERT_TRUE(g1.rank() == ((wcomm.rank() % 2) ? (wcomm.rank() / 2) : MPI_UNDEFINED));
  ASSERT_EQ(g1, g2);
  ASSERT_EQ(g2, g3);
  ASSERT_TRUE(g4.size() == wg.size() - 1);
  ASSERT_TRUE(g4.rank() == MPI_UNDEFINED);
}

TEST_F(GroupFixture, intersection)
{
  auto const g1 = wg.incl(gMembers.cbegin(), gMembers.cend()),
             g2 = wg.excl(gMembers.cbegin(), gMembers.cend()),
             g3 = g1.intersection(g2);
  ASSERT_TRUE(g3.size() == 0);
  ASSERT_TRUE(g3.rank() == MPI_UNDEFINED);
}

TEST_F(GroupFixture, join)
{
  auto const g1 = wg.incl(gMembers.cbegin(), gMembers.cend()),
             g2 = wg.excl(gMembers.cbegin(), gMembers.cend()),
             g4 = g1.join(g2);
  ASSERT_TRUE(g4.size() == wcomm.size());
  ASSERT_FALSE(g4.rank() == MPI_UNDEFINED);
}
