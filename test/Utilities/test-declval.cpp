#include "hep_hpc/Utilities/demangle_symbol.hpp"

#include "gtest/gtest.h"

#include <string>
#include <typeinfo>

namespace {
  template <typename FUNC, typename... ARGS>
  auto
  callit(FUNC func, ARGS && ... args)
-> decltype(func(std::forward<ARGS>(args)...))
  {
    decltype(func(std::forward<ARGS>(args)...)) result;
    result = func(std::forward<ARGS>(args)...);
    return result;
  }
}

TEST(test_declval, test_declval)
{
  using namespace std::string_literals;
  auto y = callit([](int i, int j, double x) -> double { return x * (i + j); }, 6, 5, 3.4);
  ASSERT_EQ(hep_hpc::detail::demangle_symbol(typeid(y).name()), "double"s);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
