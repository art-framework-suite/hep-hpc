#include "hep_hpc/hdf5/Dataset.hpp"
#include "hep_hpc/hdf5/Dataspace.hpp"
#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/ScopedErrorHandler.hpp"

#include "gtest/gtest.h"

#include <string>

using namespace hep_hpc::hdf5;
using namespace std::string_literals;

namespace {
  PropertyList fileAccessProperties()
  {
    // Ensure we are using the latest available HDF5 file format to write our data.
    PropertyList plist(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(plist, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    return plist;
  }
}

TEST(Dataset, def)
{
  Dataset const s;
  ASSERT_FALSE(s);
}

TEST(Dataset, construct)
{
  File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  Dataset s(h, "/D1"s, H5T_NATIVE_INT);
  ASSERT_TRUE(s);
}

TEST(Dataset, move_construction)
{
  File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  Dataset s(h, "/D1"s, H5T_NATIVE_INT);
  ASSERT_TRUE(s);
  Dataset const s2(std::move(s));
  ASSERT_FALSE(s);
  ASSERT_TRUE(s2);
}

TEST(Dataset, move_assignment)
{
  File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  Dataset s(h, "/D1"s, H5T_NATIVE_INT);
  ASSERT_TRUE(s);
  Dataset s2;
  s2 = std::move(s);
  ASSERT_FALSE(s);
  ASSERT_TRUE(s2);
}

TEST(Dataset, open_success)
{
  File h("h5dset_t.hdf5"s);
  Dataset s(h, "/D1"s);
  ASSERT_TRUE(s);
}

TEST(Dataset, open_missing)
{
  File h("h5dset_t.hdf5"s);
  ScopedErrorHandler seh;
  Dataset s(h, "/G2"s);
  ASSERT_FALSE(s);
}

TEST(Dataset, write)
{
  File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  Dataset s(h, "/G1"s, H5T_NATIVE_INT);
  int const i{5};
  ASSERT_EQ(s.write(H5T_NATIVE_INT, &i), (herr_t)0);
}

TEST(Dataset, read)
{
  File const h("h5dset_t.hdf5"s);
  Dataset s(h, "/G1"s);
  int i{12};
  ASSERT_EQ(s.read(H5T_NATIVE_INT, &i), (herr_t)0);
  ASSERT_EQ(i, 5);
}

TEST(Dataset, flush)
{
  File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  Dataset s(h, "/G1"s, H5T_NATIVE_INT);
  ASSERT_TRUE(s);
  ASSERT_EQ(s.flush(), (herr_t)0);
  ASSERT_TRUE(s);
}

TEST(Dataset, refresh)
{
  File const h("h5dset_t.hdf5"s, H5F_ACC_RDWR);
  Dataset s(h, "/G1"s);
  ASSERT_TRUE(s);
  ASSERT_EQ(s.refresh(), (herr_t)0);
  ASSERT_TRUE(s);
}

TEST(Dataset, reset)
{
  File const h("h5dset_t.hdf5"s, H5F_ACC_RDWR);
  Dataset s(h, "/G1"s);
  ASSERT_TRUE(s);
  s.reset();
  ASSERT_FALSE(s);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  setErrorHandler(ErrorMode::EXCEPTIONS);
  return RUN_ALL_TESTS();
}
