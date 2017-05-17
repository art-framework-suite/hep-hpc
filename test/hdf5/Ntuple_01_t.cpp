#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/make_ntuple.hpp"

using namespace hep_hpc::hdf5;

#include <iostream>
#include <string>
#include <vector>

int main()
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  auto data = make_ntuple({"test-ntuple.hdf5", "g1",  2},
                          make_column<int>("A", 2));
  int i1data[] = { 1, 1, 2, 4, 3, 6, 5, 10, 7, 14, 11, 22, 13, 26, 17, 34, 23, 46};
  for (auto i = 0; i < 9; ++i) {
    data.insert(&i1data[i*2]);
  }
}
