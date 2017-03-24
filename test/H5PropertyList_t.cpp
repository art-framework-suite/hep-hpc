#include "hep_hpc/H5PropertyList.hpp"
#include "hep_hpc/initH5ErrorHandling.hpp"

#include "gtest/gtest.h"

#include <iostream>

using namespace hep_hpc;
using namespace std::string_literals;

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

TEST(H5PropertyList, default_construction)
{
  H5PropertyList const plist;
  ASSERT_FALSE(plist);
}

TEST(H5PropertyList, construction)
{
  H5PropertyList const plist(H5P_FILE_ACCESS);
  ASSERT_TRUE(plist);
}

TEST(H5PropertyList, copy_construction)
{
  H5PropertyList const plist1(H5P_FILE_ACCESS);
  H5PropertyList const plist2(plist1);
  ASSERT_TRUE(plist1);
  ASSERT_TRUE(plist2);
  ASSERT_NE(plist1, plist2);
  ASSERT_EQ(plist1.getClass(), plist2.getClass());
  ASSERT_EQ(plist1.getClassName(), plist2.getClassName());
}

TEST(H5PropertyList, copy_assignment)
{
  H5PropertyList const plist1(H5P_FILE_ACCESS);
  H5PropertyList plist2;
  ASSERT_FALSE(plist2);
  plist2 = plist1;
  ASSERT_TRUE(plist1);
  ASSERT_TRUE(plist2);
  ASSERT_NE(plist1, plist2);
  ASSERT_EQ(plist1.getClass(), plist2.getClass());
  ASSERT_EQ(plist1.getClassName(), plist2.getClassName());
}

TEST(H5PropertyList, move_construction)
{
  H5PropertyList plist1(H5P_FILE_ACCESS);
  H5PropertyList plist2(std::move(plist1));
  ASSERT_TRUE(plist2);
  ASSERT_FALSE(plist1);
  ASSERT_EQ(plist2.getClass(), H5P_FILE_ACCESS);
  ASSERT_EQ(plist2.getClassName(), "file access"s);
}

TEST(H5PropertyList, move_assignment)
{
  H5PropertyList plist1(H5P_FILE_ACCESS);
  H5PropertyList plist2;
  ASSERT_FALSE(plist2);
  plist2 = std::move(plist1);
  ASSERT_TRUE(plist2);
  ASSERT_FALSE(plist1);
  ASSERT_EQ(plist2.getClass(), H5P_FILE_ACCESS);
  ASSERT_EQ(plist2.getClassName(), "file access"s);
}

TEST(H5PropertyList, getClass)
{
  for (auto pclass : known_classes) {
    H5PropertyList plist(pclass);
    ASSERT_EQ(plist.getClass(), pclass);
  }
}

TEST(H5PropertyList, getClassName)
{
  std::set<std::string> seen_names;
  for (auto pclass : known_classes) {
    ASSERT_TRUE(seen_names.insert(H5PropertyList(pclass).getClassName()).second);
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  initH5ErrorHandling();
  return RUN_ALL_TESTS();
}
