#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/make_ntuple.hpp"

using namespace hep_hpc::hdf5;

#include <string>
#include <vector>

int main()
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  auto data =
    make_ntuple({"test-ntuple_03.hdf5", "g1"},
                make_column<int, 2>("A", {2, 3}),
                make_scalar_column<double>("B",
                  {PropertyList{H5P_DATASET_CREATE}
                    (&H5Pset_shuffle)
                    (&H5Pset_deflate, 7u)}));
  int i1data[] = {  1,  2,  3,  4,  5,  6,
                    7,  8,  9, 10, 11, 12,
                   13, 14, 15, 16, 17, 18};
  double d1data[] = { 1.01, 2.02, 3.03 };
  for (auto i = 0; i < 3; ++i) {
    data.insert(&i1data[i*6], d1data[i]);
  }
}
