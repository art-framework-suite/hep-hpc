#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/Group.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "gtest/gtest.h"

#include <string>

using namespace hep_hpc::hdf5;

namespace {
  PropertyList fileAccessProperties()
  {
    // Ensure we are using the latest available HDF5 file format to write our data.
    PropertyList plist(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(plist, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    return plist;
  }
}

TEST(Group, def)
{
  Group const g;
  ASSERT_FALSE(g);
}

TEST(Group, construct)
{
  File const h("h5group_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  Group g(h, "/G1");
  ASSERT_TRUE(g);
}

TEST(Group, move_construction)
{
  File const h("h5group_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  Group g(h, "/G1");
  ASSERT_TRUE(g);
  Group const g2(std::move(g));
  ASSERT_FALSE(g);
  ASSERT_TRUE(g2);
}

TEST(Group, move_assignment)
{
  File const h("h5group_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  Group g(h, "/G1");
  ASSERT_TRUE(g);
  Group g2;
  g2 = std::move(g);
  ASSERT_FALSE(g);
  ASSERT_TRUE(g2);
}

TEST(Group, open_success)
{
  File h("h5group_t.hdf5");
  Group g(h, "/G1", Group::OPEN_MODE);
  ASSERT_TRUE(g);
}

TEST(Group, open_missing)
{
  File h("h5group_t.hdf5");
  ScopedErrorHandler seh;
  Group g(h, "/G2", Group::OPEN_MODE);
  ASSERT_FALSE(g);
}

TEST(Group, open_or_create)
{
  File h("h5group_t.hdf5", H5F_ACC_RDWR);
  Group g(h, "/G2", Group::OPEN_OR_CREATE_MODE);
  ASSERT_TRUE(g);
}

TEST(Group, info)
{
  File h("h5group_t.hdf5", H5F_ACC_RDWR);
  Group g(h, "/G1", Group::OPEN_MODE);
  ASSERT_TRUE(g);
  ASSERT_EQ(g.info().nlinks, 0ull);
  Group g3(g, "G3", Group::CREATE_MODE);
  ASSERT_TRUE(g3);
  ASSERT_EQ(g.info().nlinks, 1ull);
}

TEST(Group, flush)
{
  File const h("h5group_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  Group g(h, "/G1");
  ASSERT_TRUE(g);
  ASSERT_EQ(g.flush(), (herr_t)0);
  ASSERT_TRUE(g);
}

TEST(Group, refresh)
{
  File const h("h5group_t.hdf5", H5F_ACC_RDWR);
  Group g(h, "/G1", Group::OPEN_MODE);
  ASSERT_TRUE(g);
  ASSERT_EQ(g.refresh(), (herr_t)0);
  ASSERT_TRUE(g);
}

TEST(Group, reset)
{
  File const h("h5group_t.hdf5", H5F_ACC_RDWR);
  Group g(h, "/G1", Group::OPEN_MODE);
  ASSERT_TRUE(g);
  g.reset();
  ASSERT_FALSE(g);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  return RUN_ALL_TESTS();
}
