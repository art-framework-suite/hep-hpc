#include "hep_hpc/Ntuple.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

using namespace hep_hpc;
using namespace hep_hpc::hdf5;

int main()
{
  using namespace std::string_literals;
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  Ntuple<int, double, Column<int, 2> > data("test-ntuple.hdf5", "g1",
                                            {{"I1"s, 2}, "D1"s, {"I2"s, {2, 3}}},
                                            2);
  int i1data[] = { 1, 1, 2, 4, 3, 6, 5, 10, 7, 14, 11, 22, 13, 26, 17, 34, 23, 46};
  double d1data[] = { 1.01, 2.02, 3.03, 5.05, 7.07, 11.11, 13.13, 19.17, 23.23 };
  int i2data[] = { 0,   1,  2,  5,  6,  7,
                   10, 11, 12, 15, 16, 17,
                   20, 21, 22, 25, 26, 27,
                   30, 31, 32, 35, 36, 37,
                   40, 41, 42, 45, 46, 47,
                   50, 51, 52, 55, 56, 57,
                   60, 61, 62, 65, 66, 67,
                   70, 71, 72, 75, 76, 77,
                   80, 81, 82, 85, 86, 87 };
  for (auto i = 0; i < 9; ++i) {
    data.insert(&i1data[i*2], &d1data[i], &i2data[i*6]);
  }
}
