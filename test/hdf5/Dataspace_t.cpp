#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "gtest/gtest.h"

#include <array>
#include <list>

using namespace hep_hpc::hdf5;

TEST(Dataspace, default_construction)
{
  Dataspace const dspace;
  ASSERT_TRUE(dspace.is_valid());
  ASSERT_TRUE(dspace.is_default());
  ASSERT_FALSE(dspace.is_valid_non_default());
}

TEST(Dataspace, construction_from_class)
{
  Dataspace const dspace(H5S_NULL);
  ASSERT_TRUE(dspace.is_valid_non_default());
}

TEST(Dataspace, construction_from_pointers)
{
  hsize_t const dims[] = { 2, 3, 5 };
  Dataspace const dspace(3, dims, dims);
  ASSERT_TRUE(dspace.is_valid_non_default());
}

TEST(Dataspace, construction_from_iters)
{
  std::array<hsize_t, 3> dims { 2, 3, 5 };
  std::list<hsize_t> maxdims { 2, 3, 5 };
  Dataspace const dspace(3, dims.begin(), maxdims.begin());
  ASSERT_TRUE(dspace.is_valid_non_default());
  Dataspace const dspace2(dims.begin(), dims.end(),
                            maxdims.begin(), maxdims.end());
  ASSERT_TRUE(dspace2.is_valid_non_default());
}

TEST(Dataspace, copy_construction)
{
  Dataspace const dspace1(H5S_NULL);
  Dataspace const dspace2(dspace1);
  ASSERT_TRUE(dspace1.is_valid_non_default());
  ASSERT_TRUE(dspace2.is_valid_non_default());
  ASSERT_NE(dspace1, dspace2);
}

TEST(Dataspace, copy_assignment)
{
  Dataspace const dspace1(H5S_NULL);
  ASSERT_TRUE(dspace1.is_valid_non_default());
  Dataspace dspace2;
  ASSERT_TRUE(dspace2.is_default());
  dspace2 = dspace1;
  ASSERT_TRUE(dspace2.is_valid_non_default());
  ASSERT_NE(dspace1, dspace2);
}

TEST(Dataspace, move_construction)
{
  Dataspace dspace1(H5S_NULL);
  Dataspace dspace2(std::move(dspace1));
  ASSERT_TRUE(dspace2.is_valid_non_default());
}

TEST(Dataspace, move_assignment)
{
  Dataspace dspace1(H5S_NULL);
  Dataspace dspace2;
  ASSERT_TRUE(dspace1.is_valid_non_default());
  ASSERT_TRUE(dspace2.is_default());
  dspace2 = std::move(dspace1);
  ASSERT_TRUE(dspace2.is_valid_non_default());
}

TEST(Dataspace, reset)
{
  Dataspace dspace(H5S_NULL);
  ASSERT_TRUE(dspace.is_valid_non_default());
  dspace.reset();
  ASSERT_TRUE(dspace.is_default());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  return RUN_ALL_TESTS();
}
