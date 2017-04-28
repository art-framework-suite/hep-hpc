#include "hep_hpc/hdf5/Column.hpp"

using namespace hep_hpc::hdf5;

#include <array>
#include <string>

using namespace std::string_literals;

int main()
{
  Column<int> x("X"s);
  Column<int> y("Y"s, 3);
  Column<int, 2> z("Z"s, {3, 2});
  Column<std::array<char, 5> > f1("F1"s);
  Column<std::array<char, 5>, 2> f2("F1"s, {1, 2});
}
