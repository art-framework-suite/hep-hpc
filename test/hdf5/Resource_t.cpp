#include "hep_hpc/hdf5/Resource.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"
#include "gtest/gtest.h"

#include <string>

using namespace hep_hpc::hdf5;

TEST(Resource, construct_empty)
{
  Resource<hid_t> const r;
  ASSERT_EQ(*r, 0ll);
}

TEST(Resource, construct_non_owning)
{
  HID_t const ref(27ll);
  Resource<HID_t> const r(ref);
  ASSERT_EQ(*r, ref);
}

TEST(Resource, construct_simple)
{
  hid_t const ref(31ll);
  HID_t iut;
  {
    Resource<hid_t>(ref, [&iut](hid_t rh) { iut = rh; return 0;});
  }
  ASSERT_EQ((hid_t)iut, ref);
}

TEST(Resource, construct_complex)
{
  hid_t const ref(31ll);
  HID_t iut;
  {
    Resource<hid_t>([](auto h) { return h; }, [&iut](hid_t rh) { iut = rh; return iut;}, ref);
  }
  ASSERT_EQ((hid_t)iut, ref);
}

TEST(Resource, move_construct)
{
  HID_t const ref {27ll};
  Resource<HID_t> r1{ref, [](auto){return 0;}};
  Resource<HID_t> r2(std::move(r1));
  ASSERT_FALSE(r1.teardownFunc());
  ASSERT_EQ(*r1, HID_t {});
  ASSERT_TRUE(r2.teardownFunc());
  ASSERT_EQ(*r2, ref); 
}

TEST(Resource, move_assign)
{
  HID_t const ref {27ll};
  Resource<HID_t> r1{ref, [](auto){return 0;}};
  Resource<HID_t> r2;
  r2 = std::move(r1);
  ASSERT_FALSE(r1.teardownFunc());
  ASSERT_EQ(*r1, HID_t {});
  ASSERT_TRUE(r2.teardownFunc());
  ASSERT_EQ(*r2, ref); 
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  return RUN_ALL_TESTS();
}
