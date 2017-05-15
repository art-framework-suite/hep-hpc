#include "hep_hpc/hdf5/Ntuple.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

using namespace hep_hpc::hdf5;

#include <string>
#include <vector>

int main()
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  Ntuple<double> data("test-ntuple.hdf5", "g1", {"B"}, 2);
  double d1data[] = { 1.01, 2.02, 3.03, 5.05, 7.07, 11.11, 13.13, 19.17, 23.23 };
  for (auto i = 0; i < 9; ++i) {
    data.insert(d1data[i]);
  }
}
