#include "hep_hpc/H5File.hpp"

#include "gtest/gtest.h"

using namespace hep_hpc;

TEST(H5File, def)
{
  H5File h;
  ASSERT_FALSE(h);
}

TEST(H5File, construct)
{
  H5File h("empty.hdf5");
  ASSERT_TRUE(h);
}

TEST(H5File, move)
{
  H5File h("empty.hdf5");
  H5File h2(std::move(h));
  ASSERT_FALSE(h);
  ASSERT_TRUE(h2);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
