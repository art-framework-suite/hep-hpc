#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/Group.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/write_attribute.hpp"

#include "gtest/gtest.h"

#include <string>

using namespace hep_hpc::hdf5;

namespace {
  PropertyList fileAccessProperties()
  {
    // Ensure we are using the latest available HDF5 file format to
    // write our data.
    PropertyList plist(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(plist, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    return plist;
  }
}

TEST(write_attribute, Group)
{
  File h("write_attribute_group_t.hdf5",
         H5F_ACC_TRUNC, {}, fileAccessProperties());
  Group g_m(h, "MetaData", Group::CREATE_MODE);
  Group g_r(g_m, "Run077625", Group::CREATE_MODE);
  Group g_sr(g_r, "SubRun002", Group::CREATE_MODE);
  write_attribute(g_sr, "JSON", "Lots\nOf\nStuff");
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  return RUN_ALL_TESTS();
}
