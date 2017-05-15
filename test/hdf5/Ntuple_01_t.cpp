#include "hep_hpc/hdf5/Ntuple.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

using namespace hep_hpc::hdf5;

#include <string>
#include <vector>

int main()
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  Ntuple<int> data("test-ntuple.hdf5", "g1",  Ntuple<int>::column_info_t {{"A", 2}}, 2);
  int i1data[] = { 1, 1, 2, 4, 3, 6, 5, 10, 7, 14, 11, 22, 13, 26, 17, 34, 23, 46};
  for (auto i = 0; i < 9; ++i) {
    data.insert(&i1data[i*2]);
  }
}
