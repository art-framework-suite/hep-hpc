#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/Column.hpp"
#include "hep_hpc/hdf5/Ntuple.hpp"

using namespace hep_hpc::hdf5;

#include <iostream>
#include <string>
#include <vector>

int main()
{
  Ntuple<int, double> nt("test-ntuple_05.hdf5", "data",
                         {"Int_Data", "Double_Data"});
  int idata[] = { 1, 2, 3 };
  double ddata[] = { 4.5, 5.5, 6.5 };
  for (auto i = 0; i < 3; ++i) {
    nt.insert(idata[i], ddata[i]);
  }
}
