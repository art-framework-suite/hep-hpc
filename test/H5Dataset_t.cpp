#include "hep_hpc/H5Dataset.hpp"
#include "hep_hpc/H5Dataspace.hpp"
#include "hep_hpc/H5File.hpp"
#include "hep_hpc/H5PropertyList.hpp"
#include "hep_hpc/errorHandling.hpp"
#include "hep_hpc/ScopedErrorHandler.hpp"

#include "gtest/gtest.h"

#include <string>

using namespace hep_hpc;
using namespace std::string_literals;

namespace {
  hep_hpc::H5PropertyList fileAccessProperties()
  {
    // Ensure we are using the latest available HDF5 file format to write our data.
    hep_hpc::H5PropertyList plist(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(plist, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    return plist;
  }
}

TEST(H5Dataset, def)
{
  H5Dataset const s;
  ASSERT_FALSE(s);
}

TEST(H5Dataset, construct)
{
  H5File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5Dataset s(h, "/D1"s, H5T_NATIVE_INT);
  ASSERT_TRUE(s);
}

TEST(H5Dataset, move_construction)
{
  H5File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5Dataset s(h, "/D1"s, H5T_NATIVE_INT);
  ASSERT_TRUE(s);
  H5Dataset const s2(std::move(s));
  ASSERT_FALSE(s);
  ASSERT_TRUE(s2);
}

TEST(H5Dataset, move_assignment)
{
  H5File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5Dataset s(h, "/D1"s, H5T_NATIVE_INT);
  ASSERT_TRUE(s);
  H5Dataset s2;
  s2 = std::move(s);
  ASSERT_FALSE(s);
  ASSERT_TRUE(s2);
}

TEST(H5Dataset, open_success)
{
  H5File h("h5dset_t.hdf5"s);
  H5Dataset s(h, "/D1"s);
  ASSERT_TRUE(s);
}

TEST(H5Dataset, open_missing)
{
  H5File h("h5dset_t.hdf5"s);
  ScopedErrorHandler seh;
  H5Dataset s(h, "/G2"s);
  ASSERT_FALSE(s);
}

TEST(H5Dataset, write)
{
  H5File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5Dataset s(h, "/G1"s, H5T_NATIVE_INT);
  int const i{5};
  ASSERT_EQ(s.write(H5T_NATIVE_INT, &i), (herr_t)0);
}

TEST(H5Dataset, read)
{
  H5File const h("h5dset_t.hdf5"s);
  H5Dataset s(h, "/G1"s);
  int i{12};
  ASSERT_EQ(s.read(H5T_NATIVE_INT, &i), (herr_t)0);
  ASSERT_EQ(i, 5);
}

TEST(H5Dataset, flush)
{
  H5File const h("h5dset_t.hdf5"s, H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5Dataset s(h, "/G1"s, H5T_NATIVE_INT);
  ASSERT_TRUE(s);
  ASSERT_EQ(s.flush(), (herr_t)0);
  ASSERT_TRUE(s);
}

TEST(H5Dataset, refresh)
{
  H5File const h("h5dset_t.hdf5"s, H5F_ACC_RDWR);
  H5Dataset s(h, "/G1"s);
  ASSERT_TRUE(s);
  ASSERT_EQ(s.refresh(), (herr_t)0);
  ASSERT_TRUE(s);
}

TEST(H5Dataset, reset)
{
  H5File const h("h5dset_t.hdf5"s, H5F_ACC_RDWR);
  H5Dataset s(h, "/G1"s);
  ASSERT_TRUE(s);
  s.reset();
  ASSERT_FALSE(s);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  setErrorHandler(ErrorMode::EXCEPTIONS);
  return RUN_ALL_TESTS();
}
