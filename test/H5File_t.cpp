#include "hep_hpc/H5File.hpp"
#include "hep_hpc/errorHandling.hpp"
#include "hep_hpc/ScopedErrorHandler.hpp"

#include "gtest/gtest.h"

using namespace hep_hpc;

TEST(H5File, def)
{
  H5File const h;
  ASSERT_FALSE(h);
}

TEST(H5File, construct)
{
  H5File const h("empty.hdf5");
  ASSERT_TRUE(h);
}

TEST(H5File, move_construction)
{
  H5File h("empty.hdf5");
  H5File const h2(std::move(h));
  ASSERT_FALSE(h);
  ASSERT_TRUE(h2);
}

TEST(H5File, move_assignment)
{
  H5File h("empty.hdf5");
  H5File h2;
  h2 = std::move(h);
  ASSERT_FALSE(h);
  ASSERT_TRUE(h2);
}

TEST(H5File, explicit_close)
{
  H5File h("empty.hdf5");
  ASSERT_TRUE(h);
  h.close();
  ASSERT_FALSE(h);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  initH5ErrorHandling();
  return RUN_ALL_TESTS();
}
