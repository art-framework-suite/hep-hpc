#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/make_ntuple.hpp"

using namespace hep_hpc::hdf5;

#include <string>
#include <vector>

int main()
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  auto data = make_ntuple({"test-ntuple.hdf5", "g1", 2},
                          make_column<double>("B"));
  double d1data[] = { 1.01, 2.02, 3.03, 5.05, 7.07, 11.11, 13.13, 19.17, 23.23 };
  for (auto i = 0; i < 9; ++i) {
    data.insert(d1data[i]);
  }
}
