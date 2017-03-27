#include "hep_hpc/H5File.hpp"
#include "hep_hpc/H5Group.hpp"
#include "hep_hpc/errorHandling.hpp"
#include "hep_hpc/ScopedErrorHandler.hpp"

#include "gtest/gtest.h"

#include <string>

using namespace std::string_literals;

using namespace hep_hpc;

namespace {
  hep_hpc::H5PropertyList fileAccessProperties()
  {
    // Ensure we are using the latest available HDF5 file format to write our data.
    hep_hpc::H5PropertyList plist(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(plist, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    return plist;
  }
}

TEST(H5Group, def)
{
  H5Group const g;
  ASSERT_FALSE(g);
}

TEST(H5Group, construct)
{
  H5File const h("h5group_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5Group g(h, "/G1"s);
  ASSERT_TRUE(g);
}

TEST(H5Group, move_construction)
{
  H5File const h("h5group_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5Group g(h, "/G1"s);
  ASSERT_TRUE(g);
  H5Group const g2(std::move(g));
  ASSERT_FALSE(g);
  ASSERT_TRUE(g2);
}

TEST(H5Group, move_assignment)
{
  H5File const h("h5group_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5Group g(h, "/G1"s);
  ASSERT_TRUE(g);
  H5Group g2;
  g2 = std::move(g);
  ASSERT_FALSE(g);
  ASSERT_TRUE(g2);
}

TEST(H5Group, open_success)
{
  H5File h("h5group_t.hdf5");
  H5Group g(h, "/G1"s, H5Group::OPEN_MODE);
  ASSERT_TRUE(g);
}

TEST(H5Group, open_missing)
{
  H5File h("h5group_t.hdf5");
  ScopedErrorHandler seh;
  H5Group g(h, "/G2"s, H5Group::OPEN_MODE);
  ASSERT_FALSE(g);
}

TEST(H5Group, open_or_create)
{
  H5File h("h5group_t.hdf5", H5F_ACC_RDWR);
  H5Group g(h, "/G2"s, H5Group::OPEN_OR_CREATE_MODE);
  ASSERT_TRUE(g);
}

TEST(H5Group, info)
{
  H5File h("h5group_t.hdf5", H5F_ACC_RDWR);
  H5Group g(h, "/G1"s, H5Group::OPEN_MODE);
  ASSERT_TRUE(g);
  ASSERT_EQ(g.info().nlinks, 0ull);
  H5Group g3(g, "G3"s, H5Group::CREATE_MODE);
  ASSERT_TRUE(g3);
  ASSERT_EQ(g.info().nlinks, 1ull);
}

TEST(H5Group, flush)
{
  H5File const h("h5group_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5Group g(h, "/G1"s);
  ASSERT_TRUE(g);
  g.flush();
  ASSERT_TRUE(g);
}

TEST(H5Group, refresh)
{
  H5File const h("h5group_t.hdf5", H5F_ACC_RDWR);
  H5Group g(h, "/G1"s, H5Group::OPEN_MODE);
  ASSERT_TRUE(g);
  g.refresh();
  ASSERT_TRUE(g);
}

TEST(H5Group, reset)
{
  H5File const h("h5group_t.hdf5", H5F_ACC_RDWR);
  H5Group g(h, "/G1"s, H5Group::OPEN_MODE);
  ASSERT_TRUE(g);
  g.reset();
  ASSERT_FALSE(g);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  initH5ErrorHandling();
  return RUN_ALL_TESTS();
}
