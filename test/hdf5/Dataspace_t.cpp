#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "gtest/gtest.h"

#include <array>
#include <list>

using namespace hep_hpc::hdf5;
using namespace std::string_literals;

TEST(Dataspace, default_construction)
{
  Dataspace const dspace;
  ASSERT_FALSE(dspace);
}

TEST(Dataspace, construction_from_class)
{
  Dataspace const dspace(H5S_NULL);
  ASSERT_TRUE(dspace);
}

TEST(Dataspace, construction_from_pointers)
{
  hsize_t const dims[] = { 2, 3, 5 };
  Dataspace const dspace(3, dims, dims);
  ASSERT_TRUE(dspace);
}

TEST(Dataspace, construction_from_iters)
{
  std::array<hsize_t, 3> dims { 2, 3, 5 };
  std::list<hsize_t> maxdims { 2, 3, 5 };
  Dataspace const dspace(3, dims.begin(), maxdims.begin());
  ASSERT_TRUE(dspace);
  Dataspace const dspace2(dims.begin(), dims.end(),
                            maxdims.begin(), maxdims.end());
  ASSERT_TRUE(dspace2);
}

TEST(Dataspace, copy_construction)
{
  Dataspace const dspace1(H5S_NULL);
  Dataspace const dspace2(dspace1);
  ASSERT_TRUE(dspace1);
  ASSERT_TRUE(dspace2);
  ASSERT_NE(dspace1, dspace2);
}

TEST(Dataspace, copy_assignment)
{
  Dataspace const dspace1(H5S_NULL);
  Dataspace dspace2;
  ASSERT_FALSE(dspace2);
  dspace2 = dspace1;
  ASSERT_TRUE(dspace1);
  ASSERT_TRUE(dspace2);
  ASSERT_NE(dspace1, dspace2);
}

TEST(Dataspace, move_construction)
{
  Dataspace dspace1(H5S_NULL);
  Dataspace dspace2(std::move(dspace1));
  ASSERT_TRUE(dspace2);
  ASSERT_FALSE(dspace1);
}

TEST(Dataspace, move_assignment)
{
  Dataspace dspace1(H5S_NULL);
  Dataspace dspace2;
  ASSERT_FALSE(dspace2);
  dspace2 = std::move(dspace1);
  ASSERT_TRUE(dspace2);
  ASSERT_FALSE(dspace1);
}

TEST(Dataspace, reset)
{
  Dataspace dspace(H5S_NULL);
  ASSERT_TRUE(dspace);
  dspace.reset();
  ASSERT_FALSE(dspace);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  return RUN_ALL_TESTS();
}
