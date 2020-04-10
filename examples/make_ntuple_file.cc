#include "hep_hpc/hdf5/Column.hpp"
#include "hep_hpc/hdf5/Ntuple.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/make_column.hpp"
#include "hep_hpc/hdf5/make_ntuple.hpp"

#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace hep_hpc::hdf5;
using std::string;
using std::vector;

int main(int argc, char *argv[]) {
  vector<string> args(argv + 1, argv + argc);
  if (args.size() != 3) {
    std::cout << "specify an output filename and two non-negative integers, the number of rows for the two tables\n";
    return 1;
  }

  string filename = args[0];
  std::size_t nrows_1 = std::atol(args[1].c_str());
  std::size_t nrows_2 = std::atol(args[2].c_str());

  // Open the HDF5 file into which we will write our data.
  File output(filename, H5F_ACC_TRUNC); 

  // make_ntuple is the main facility for creating ntuple files.
  // make_scalar_column creates a column carrying a scalar data type.
  // For column 'a', we have used the facilities provided to set properties of
  // the dataset.
  auto nt_1 = make_ntuple(
      {output, "table_1", 1000},
      make_scalar_column<int>(
          "a",                       // dataset name
          1024 * 1024 / sizeof(int), // chunk size (in elements)
          {PropertyList{H5P_DATASET_CREATE}(&H5Pset_shuffle)(&H5Pset_deflate,6u)}),
      make_scalar_column<double>("b"), // all defaults
      make_scalar_column<double>("c"), make_scalar_column<int>("d"),
      make_column<int, 2>("arry", // 2 means each element is a 2-d array
                          {3, 4}, // extent of each array dimension
                          1024 * 1024 / (12 * sizeof(int)), // chunk size
                          {PropertyList{H5P_DATASET_CREATE}(&H5Pset_shuffle)(&H5Pset_deflate,6u)}));
  auto nt_2 = make_ntuple(
      {output, "table_2", 1000},
      make_scalar_column<double>("x"),
      make_scalar_column<double>("y"),
      make_scalar_column<int>("z"));

  std::mt19937 urng(123); // fix the seed for reproducibility
  std::uniform_int_distribution ints(0, 1000 * 1000);
  std::uniform_real_distribution reals(-100., 100.0);

  for (std::size_t i = 0; i != nrows_1; ++i) {
    std::array<int, 12> ary{ints(urng), ints(urng), ints(urng), ints(urng),
                            ints(urng), ints(urng), ints(urng), ints(urng),
                            ints(urng), ints(urng), ints(urng), ints(urng)};
    nt_1.insert(ints(urng), reals(urng), reals(urng), ints(urng), ary.data());
  }

  for (std::size_t i = 0; i != nrows_2; ++i) {
    nt_2.insert(reals(urng), reals(urng), ints(urng));
  }

  // ntuple is finalized and associated file is closed by the d'tor
  // of Ntuple.
}
