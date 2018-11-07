#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/Column.hpp"
#include "hep_hpc/hdf5/Ntuple.hpp"
#include "hep_hpc/hdf5/make_column.hpp"

using namespace hep_hpc::hdf5;

#include <iostream>
#include <string>
#include <vector>

using nt_t = Ntuple<Column<unsigned short, 2>,
                    unsigned short,
                    float,
                    float>;

int main()
{
  nt_t nt {"test-ntuple_06.hdf5",
      "data",
      {make_column<unsigned short, 2>("adc", {5, 10}, 200,
        {PropertyList{H5P_DATASET_CREATE}}),
        {"channelID", 240},
          make_column<float>("pedestalMean", 240, 200),
            make_column<float>("pedestalSigma", 240, 200)},
      10000};
}
