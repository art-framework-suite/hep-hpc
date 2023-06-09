#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/make_ntuple.hpp"

using namespace hep_hpc::hdf5;

#include <string>
#include <vector>

int main()
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  auto data = make_ntuple({"test-ntuple_04.hdf5", "g1", 2},
                          make_column<int, 2>("C", {2, 3}));
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
    data.insert(&i2data[i*6]);
  }
}
