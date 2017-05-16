#include "hep_hpc/hdf5/Column.hpp"

using namespace hep_hpc::hdf5;

#include <array>
#include <string>


int main()
{
  Column<int> x("X");
  Column<int> y("Y", 3);
  Column<int, 2> z("Z", {3, 2});
  Column<std::array<char, 5> > f1("F1");
  Column<std::array<char, 5>, 2> f2("F1", {1, 2});
}
