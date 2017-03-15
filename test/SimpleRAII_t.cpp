#include "hep_hpc/SimpleRAII.hpp"

#include <iostream>
#include <string>
#include <utility>

using namespace hep_hpc;

int main()
{
  using std::swap;
  // Generic silly cleanup function.
  auto setup_a = [](std::string const & s) noexcept { return s.length(); };
  auto teardown_ab =
    [](auto x) noexcept { std::cout
                          << "Cleanup of item with handle "
                          << x
                          << std::endl; };

  // Saved resource handle is an int.
  SimpleRAII<int>
    raii_a(setup_a, teardown_ab, "Antidisestablishmentarianism"),
    raii_a2,
    raii_a3(setup_a, teardown_ab, "Internationalization");
  auto & c_raii_a = raii_a;
  std::cout << "raii_a's resource: "
            << *raii_a
            << " ("
            << *c_raii_a
            << ")."
            << std::endl;
  raii_a2 = std::move(raii_a);
  *raii_a = 17; // Shouldn't see this being cleaned up!
  swap(raii_a2, raii_a3);
  // Setup function takes void.
  SimpleRAII<int>
    raii_b([]() noexcept { return 21; }, teardown_ab);
  // Setup and cleanup functions return void.
  auto setup_c = []() noexcept { std::cout << "Set up stuff." << std::endl; };
  SimpleRAII<void>
    raii_c(setup_c,
           []() noexcept { std::cout << "Tear down stuff." << std::endl; }),
    raii_c2,
    raii_c3(setup_c,
            []() noexcept { std::cout << "Tear down moar stuff." << std::endl; });
  raii_c2 = std::move(raii_c);
  swap(raii_c2, raii_c3);
}
