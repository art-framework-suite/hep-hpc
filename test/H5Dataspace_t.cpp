#include "hep_hpc/H5Dataspace.hpp"
#include "hep_hpc/errorHandling.hpp"

#include "gtest/gtest.h"

#include <array>
#include <iostream>
#include <list>

using namespace hep_hpc;
using namespace std::string_literals;

TEST(H5Dataspace, default_construction)
{
  H5Dataspace const dspace;
  ASSERT_FALSE(dspace);
}

TEST(H5Dataspace, construction_from_class)
{
  H5Dataspace const dspace(H5S_NULL);
  ASSERT_TRUE(dspace);
}

TEST(H5Dataspace, construction_from_pointers)
{
  hsize_t const dims[] = { 2, 3, 5 };
  H5Dataspace const dspace(3, dims, dims);
  ASSERT_TRUE(dspace);
}

TEST(H5Dataspace, construction_from_iters)
{
  std::array<hsize_t, 3> dims { 2, 3, 5 };
  std::list<hsize_t> maxdims { 2, 3, 5 };
  H5Dataspace const dspace(3, dims.begin(), maxdims.begin());
  ASSERT_TRUE(dspace);
  H5Dataspace const dspace2(dims.begin(), dims.end(),
                            maxdims.begin(), maxdims.end());
  ASSERT_TRUE(dspace2);
}

TEST(H5Dataspace, copy_construction)
{
  H5Dataspace const dspace1(H5S_NULL);
  H5Dataspace const dspace2(dspace1);
  ASSERT_TRUE(dspace1);
  ASSERT_TRUE(dspace2);
  ASSERT_NE(dspace1, dspace2);
}

TEST(H5Dataspace, copy_assignment)
{
  H5Dataspace const dspace1(H5S_NULL);
  H5Dataspace dspace2;
  ASSERT_FALSE(dspace2);
  dspace2 = dspace1;
  ASSERT_TRUE(dspace1);
  ASSERT_TRUE(dspace2);
  ASSERT_NE(dspace1, dspace2);
}

TEST(H5Dataspace, move_construction)
{
  H5Dataspace dspace1(H5S_NULL);
  H5Dataspace dspace2(std::move(dspace1));
  ASSERT_TRUE(dspace2);
  ASSERT_FALSE(dspace1);
}

TEST(H5Dataspace, move_assignment)
{
  H5Dataspace dspace1(H5S_NULL);
  H5Dataspace dspace2;
  ASSERT_FALSE(dspace2);
  dspace2 = std::move(dspace1);
  ASSERT_TRUE(dspace2);
  ASSERT_FALSE(dspace1);
}

TEST(H5Dataspace, reset)
{
  H5Dataspace dspace(H5S_NULL);
  ASSERT_TRUE(dspace);
  dspace.reset();
  ASSERT_FALSE(dspace);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  initH5ErrorHandling();
  return RUN_ALL_TESTS();
}
