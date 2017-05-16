#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
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

TEST(File, def)
{
  File const h;
  ASSERT_FALSE(h);
}

TEST(File, construct)
{
  File const h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  ASSERT_TRUE(h);
}

TEST(File, construct_non_owning)
{
  // Going out of scope should not throw.
  File const h((hid_t) 27);
  ASSERT_TRUE(h);
}

TEST(File, move_construction)
{
  File h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  File const h2(std::move(h));
  ASSERT_FALSE(h);
  ASSERT_TRUE(h2);
}

TEST(File, move_assignment)
{
  File h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  File h2;
  h2 = std::move(h);
  ASSERT_FALSE(h);
  ASSERT_TRUE(h2);
}

TEST(File, open_sucess)
{
  File h("h5file_t.hdf5");
  ASSERT_TRUE(h);
}

TEST(File, open_missing)
{
  ScopedErrorHandler seh;
  File h("h5file_t_noFile.hdf5");
  ASSERT_FALSE(h);
}

TEST(File, flush)
{
  File h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  ASSERT_EQ(h.flush(), herr_t(0));
}

TEST(File, explicit_close)
{
  File h("h5file_t.hdf5", H5F_ACC_TRUNC, {}, fileAccessProperties());
  ASSERT_TRUE(h);
  h.close();
  ASSERT_FALSE(h);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  return RUN_ALL_TESTS();
}
