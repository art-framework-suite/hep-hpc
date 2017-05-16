#include "hep_hpc/Utilities/SimpleRAII.hpp"

#include "gtest/gtest.h"

#include <sstream>
#include <string>
#include <utility>

using namespace hep_hpc;
using namespace hep_hpc::detail;

namespace {
  template <typename RH>
  void test_def()
  {
    SimpleRAII<RH> const r;
    ASSERT_EQ(*r, RH{});
    ASSERT_FALSE(r.teardownFunc());
  }
  template <>
  void test_def<void>()
  {
    SimpleRAII<void> const r;
    ASSERT_FALSE(r.teardownFunc());
  }
}

TEST(SimpleRAII_construction, def_size_t)
{
  test_def<size_t>();
}

TEST(SimpleRAII_construction, def_string)
{
  test_def<std::string>();
}

TEST(SimpleRAII_construction, def_size_t_ptr)
{
  test_def<size_t *>();
}

TEST(SimpleRAII_construction, def_void)
{
  test_def<void>();
}

TEST(SimpleRAII_construction, h_td_size_t)
{
  size_t const ref = 27;
  size_t iut = 0;
  {
    SimpleRAII<size_t> const r(size_t{ref}, [&iut](size_t i) { iut = i; });
    ASSERT_EQ(*r, ref);
  }
  ASSERT_EQ(iut, ref);
}

TEST(SimpleRAII_construction, td_void)
{
  size_t const ref = 27;
  size_t iut = 0;
  {
    SimpleRAII<void> const r([ref,&iut]() { iut = ref; });
  }
  ASSERT_EQ(iut, ref);
}

namespace {
  decltype(auto) setup(std::ostream & os)
  {
    return
      [&os](std::string const & s) noexcept
    {
      os << s;
      return s.length();
    };
  }
  decltype(auto) teardown(std::ostream & os)
  {
    return
      [&os](auto x) noexcept
    {
      os << x;
    };
  }
  decltype(auto) stVoid(std::ostream &os, std::string const & msg)
  {
    return [&os, &msg](){ os << msg; };
  }
}

TEST(SimpleRAII_Simple, non_owning)
{
  size_t const ref = 27ull;
  auto iut = ref;
  {
    SimpleRAII<size_t *> const r(&iut);
    ASSERT_FALSE(r.teardownFunc());
  }
  ASSERT_EQ(iut, ref);
}

TEST(SimpleRAII_simple, size_t)
{
  using std::to_string;
  std::ostringstream os(std::ios_base::ate);
  std::string const ref = "Antidisestablishmentarianism";
  {
    SimpleRAII<size_t> const raii_a(setup(os), teardown(os), ref);
    ASSERT_EQ(os.str(), ref);
    os.str("");
  }
  ASSERT_EQ(os.str(), to_string(ref.size()));
}

TEST(SimpleRAII_simple, void)
{
  auto const sref = "Hi";
  auto const tref = "Bye";
  std::ostringstream os(std::ios_base::ate);
  {
    SimpleRAII<void> const raii_a(stVoid(os, sref), stVoid(os, tref));
    ASSERT_EQ(os.str(), sref);
    os.str("");
  }
  ASSERT_EQ(os.str(), tref);
}

TEST(SimpleRAII, swap_size_t)
{
  using std::swap;
  using std::to_string;
  std::ostringstream os(std::ios_base::ate);
  std::string const ref1 = "Antidisestablishmentarianism";
  std::string const ref2 = "Internationalization";
  {
    SimpleRAII<size_t> raii_a(setup(os), teardown(os), ref1);
    ASSERT_EQ(os.str(), ref1);
    os.str("");
    {
      SimpleRAII<size_t> raii_b(setup(os), teardown(os), ref2);
      ASSERT_EQ(os.str(), ref2);
      os.str("");
      swap(raii_a, raii_b);
    }
    ASSERT_EQ(os.str(), to_string(ref1.size()));
    os.str("");
  }
  ASSERT_EQ(os.str(), to_string(ref2.size()));
}

TEST(SimpleRAII, swap_void)
{
  using std::swap;
  std::ostringstream os(std::ios_base::ate);
  auto const sref1 = "Hi1";
  auto const tref1 = "Bye1";
  auto const sref2 = "Hi2";
  auto const tref2 = "Bye2";
  {
    SimpleRAII<void> raii_a(stVoid(os, sref1), stVoid(os, tref1));
    ASSERT_EQ(os.str(), sref1);
    os.str("");
    {
      SimpleRAII<void> raii_b(stVoid(os, sref2), stVoid(os, tref2));
      ASSERT_EQ(os.str(), sref2);
      os.str("");
      swap(raii_a, raii_b);
    }
    ASSERT_EQ(os.str(), tref1);
    os.str("");
  }
  ASSERT_EQ(os.str(), tref2);
}

TEST(SimpleRAII, move_construct_size_t)
{
  using std::to_string;
  std::ostringstream os(std::ios_base::ate);
  std::string const ref = "Antidisestablishmentarianism";
  {
    SimpleRAII<size_t> raii_a(setup(os), teardown(os), ref);
    ASSERT_EQ(os.str(), ref);
    os.str("");
    {
      SimpleRAII<size_t> const raii_a2(std::move(raii_a));
      ASSERT_TRUE(os.str().empty());
    }
    ASSERT_EQ(os.str(), to_string(ref.size()));
    os.str("");
  }
  ASSERT_TRUE(os.str().empty());
}

TEST(SimpleRAII, move_construct_void)
{
  std::ostringstream os(std::ios_base::ate);
  auto const sref = "Hi";
  auto const tref = "Bye";
  {
    SimpleRAII<void> raii_a(stVoid(os, sref), stVoid(os, tref));
    ASSERT_EQ(os.str(), sref);
    os.str("");
    {
      SimpleRAII<void> const raii_a2(std::move(raii_a));
      ASSERT_TRUE(os.str().empty());
    }
    ASSERT_EQ(os.str(), tref);
    os.str("");
  }
  ASSERT_TRUE(os.str().empty());
}

TEST(SimpleRAII, move_assign_size_t)
{
  using std::to_string;
  std::ostringstream os(std::ios_base::ate);
  std::string const ref = "Antidisestablishmentarianism";
  {
    SimpleRAII<size_t> raii_a(setup(os), teardown(os), ref);
    ASSERT_EQ(os.str(), ref);
    os.str("");
    {
      SimpleRAII<size_t> raii_a2;
      raii_a2 = std::move(raii_a);
      ASSERT_TRUE(os.str().empty());
    }
    ASSERT_EQ(os.str(), to_string(ref.size()));
    os.str("");
  }
  ASSERT_TRUE(os.str().empty());
}

TEST(SimpleRAII, move_assign_void)
{
  std::ostringstream os(std::ios_base::ate);
  std::string const ref = "Antidisestablishmentarianism";
  auto const sref = "Hi";
  auto const tref = "Bye";
  {
    SimpleRAII<void> raii_a(stVoid(os, sref), stVoid(os, tref));
    ASSERT_EQ(os.str(), sref);
    os.str("");
    {
      SimpleRAII<void> raii_a2;
      raii_a2 = std::move(raii_a);
      ASSERT_TRUE(os.str().empty());
    }
    ASSERT_EQ(os.str(), tref);
    os.str("");
  }
  ASSERT_TRUE(os.str().empty());
}

TEST(SimpleRAII, release_size_t)
{
  using std::to_string;
  std::ostringstream os(std::ios_base::ate);
  std::string const ref = "Antidisestablishmentarianism";
  {
    SimpleRAII<size_t> raii_a(setup(os), teardown(os), ref);
    ASSERT_EQ(os.str(), ref);
    os.str("");
    ASSERT_EQ(raii_a.release(), ref.size());
  }
  ASSERT_TRUE(os.str().empty());
}

TEST(SimpleRAII, release_void)
{
  auto const sref = "Hi";
  auto const tref = "Bye";
  std::ostringstream os(std::ios_base::ate);
  {
    SimpleRAII<void> raii_a(stVoid(os, sref), stVoid(os, tref));
    ASSERT_EQ(os.str(), sref);
    os.str("");
    raii_a.release();
  }
  ASSERT_TRUE(os.str().empty());
}

TEST(SimpleRAII_reset1, size_t)
{
  using std::to_string;
  std::ostringstream os(std::ios_base::ate);
  std::string const ref = "Antidisestablishmentarianism";
  {
    SimpleRAII<size_t> raii_a(setup(os), teardown(os), ref);
    ASSERT_EQ(os.str(), ref);
    os.str("");
    raii_a.reset();
    ASSERT_EQ(os.str(), to_string(ref.size()));
    os.str("");
  }
  ASSERT_TRUE(os.str().empty());
}

TEST(SimpleRAII_reset2, size_t)
{
  using std::to_string;
  std::ostringstream os(std::ios_base::ate);
  std::string const ref = "Antidisestablishmentarianism";
  size_t const r2 = 34ull;
  {
    SimpleRAII<size_t> raii_a(setup(os), teardown(os), ref);
    ASSERT_EQ(os.str(), ref);
    os.str("");
    raii_a.reset(int{r2}, teardown(os));
    ASSERT_EQ(os.str(), to_string(ref.size()));
    os.str("");
  }
  ASSERT_EQ(os.str(), to_string(r2));
}

TEST(SimpleRAII_reset1, void)
{
  auto const sref = "Hi";
  auto const tref = "Bye";
  std::ostringstream os(std::ios_base::ate);
  {
    SimpleRAII<void> raii_a(stVoid(os, sref), stVoid(os, tref));
    ASSERT_EQ(os.str(), sref);
    os.str("");
    raii_a.reset();
    ASSERT_EQ(os.str(), tref);
    os.str("");
  }
  ASSERT_TRUE(os.str().empty());
}

TEST(SimpleRAII_reset2, void)
{
  auto const sref = "Hi";
  auto const tref1 = "Bye1";
  auto const tref2 = "Bye2";
  std::ostringstream os(std::ios_base::ate);
  {
    SimpleRAII<void> raii_a(stVoid(os, sref), stVoid(os, tref1));
    ASSERT_EQ(os.str(), sref);
    os.str("");
    raii_a.reset(stVoid(os, tref2));
    ASSERT_EQ(os.str(), tref1);
    os.str("");
  }
  ASSERT_EQ(os.str(), tref2);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
