#include "hep_hpc/Ntuple.hpp"
#include "hep_hpc/errorHandling.hpp"

using namespace hep_hpc;

int main()
{
  using namespace std::string_literals;
  setErrorHandler(ErrorMode::EXCEPTIONS);
  Ntuple<int, double> data("test-ntuple.hdf5", "g1", { "I1"s, "D1"s }, 2);
  int idata[] = { 1, 2, 3, 5, 7, 11, 13, 17, 23 };
  double ddata[] = { 1.01, 2.02, 3.03, 5.05, 7.07, 11.11, 13.13, 19.17, 23.23 };
  for (auto i = 0; i < 9; ++i) {
    data.insert(idata[i], ddata[i]);
  }
}
