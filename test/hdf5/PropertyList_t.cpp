#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "gtest/gtest.h"

using namespace hep_hpc::hdf5;

namespace {
  auto const known_classes =
  {
    H5P_ATTRIBUTE_CREATE,
    H5P_DATASET_ACCESS,
    H5P_DATASET_CREATE,
    H5P_DATASET_XFER,
    H5P_DATATYPE_ACCESS,
    H5P_DATATYPE_CREATE,
    H5P_FILE_ACCESS,
    H5P_FILE_CREATE,
    H5P_FILE_MOUNT,
    H5P_GROUP_ACCESS,
    H5P_GROUP_CREATE,
    H5P_LINK_ACCESS,
    H5P_LINK_CREATE,
    H5P_OBJECT_COPY,
    H5P_OBJECT_CREATE,
    H5P_STRING_CREATE
  };
}

TEST(PropertyList, default_construction)
{
  PropertyList const plist;
  ASSERT_TRUE(plist.is_valid());
  ASSERT_TRUE(plist.is_default());
  ASSERT_FALSE(plist.is_valid_non_default());
}

TEST(PropertyList, construction)
{
  PropertyList const plist(H5P_FILE_ACCESS);
  ASSERT_TRUE(plist.is_valid());
  ASSERT_TRUE(plist.is_valid_non_default());
}

TEST(PropertyList, copy_construction)
{
  PropertyList const plist1(H5P_FILE_ACCESS);
  PropertyList const plist2(plist1);
  ASSERT_TRUE(plist1.is_valid_non_default());
  ASSERT_TRUE(plist2.is_valid_non_default());
  ASSERT_NE(plist1, plist2);
  ASSERT_EQ(plist1.getClass(), plist2.getClass());
  ASSERT_EQ(plist1.getClassName(), plist2.getClassName());
}

TEST(PropertyList, copy_assignment)
{
  PropertyList const plist1(H5P_FILE_ACCESS);
  PropertyList plist2;
  ASSERT_TRUE(plist2.is_default());
  plist2 = plist1;
  ASSERT_TRUE(plist1.is_valid_non_default());
  ASSERT_TRUE(plist2.is_valid_non_default());
  ASSERT_NE(plist1, plist2);
  ASSERT_EQ(plist1.getClass(), plist2.getClass());
  ASSERT_EQ(plist1.getClassName(), plist2.getClassName());
}

TEST(PropertyList, move_construction)
{
  PropertyList plist1(H5P_FILE_ACCESS);
  ASSERT_TRUE(plist1.is_valid_non_default());
  PropertyList plist2(std::move(plist1));
  ASSERT_TRUE(plist2.is_valid_non_default());
  ASSERT_EQ(plist2.getClass(), H5P_FILE_ACCESS);
  ASSERT_EQ(plist2.getClassName(), "file access");
}

TEST(PropertyList, move_assignment)
{
  PropertyList plist1(H5P_FILE_ACCESS);
  PropertyList plist2;
  ASSERT_TRUE(plist2.is_default());
  plist2 = std::move(plist1);
  ASSERT_TRUE(plist2.is_valid_non_default());
  ASSERT_EQ(plist2.getClass(), H5P_FILE_ACCESS);
  ASSERT_EQ(plist2.getClassName(), "file access");
}

TEST(PropertyList, getClass)
{
  for (auto pclass : known_classes) {
    PropertyList plist(pclass);
    ASSERT_EQ(plist.getClass(), pclass);
  }
}

TEST(PropertyList, getClassName)
{
  std::set<std::string> seen_names;
  for (auto pclass : known_classes) {
    ASSERT_TRUE(seen_names.insert(PropertyList(pclass).getClassName()).second);
  }
}

TEST(PropertyList, reset)
{
  PropertyList plist(H5P_FILE_ACCESS);
  ASSERT_TRUE(plist.is_valid_non_default());
  plist.reset();
  ASSERT_TRUE(plist.is_default());
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  return RUN_ALL_TESTS();
}
