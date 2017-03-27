#include "hep_hpc/H5File.hpp"
#include "hep_hpc/H5PropertyList.hpp"
#include "hep_hpc/errorHandling.hpp"
#include "hep_hpc/ScopedErrorHandler.hpp"

#include "gtest/gtest.h"

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

TEST(H5File, def)
{
  H5File const h;
  ASSERT_FALSE(h);
}

TEST(H5File, construct)
{
  H5File const h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  ASSERT_TRUE(h);
}

TEST(H5File, move_construction)
{
  H5File h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5File const h2(std::move(h));
  ASSERT_FALSE(h);
  ASSERT_TRUE(h2);
}

TEST(H5File, move_assignment)
{
  H5File h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  H5File h2;
  h2 = std::move(h);
  ASSERT_FALSE(h);
  ASSERT_TRUE(h2);
}

TEST(H5File, open_sucess)
{
  H5File h("h5file_t.hdf5");
  ASSERT_TRUE(h);
}

TEST(H5File, open_missing)
{
  ScopedErrorHandler seh;
  H5File h("h5file_t_noFile.hdf5");
  ASSERT_FALSE(h);
}

TEST(H5File, flush)
{
  H5File h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  h.flush();
}

TEST(H5File, explicit_close)
{
  H5File h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  ASSERT_TRUE(h);
  h.close();
  ASSERT_FALSE(h);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  initH5ErrorHandling();
  return RUN_ALL_TESTS();
}
