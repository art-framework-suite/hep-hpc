#include "hep_hpc/hdf5/Ntuple.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

using namespace hep_hpc::hdf5;

#include <string>
#include <vector>

int main()
{
  using namespace std::string_literals;
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  Ntuple<int, double, Column<int, 2>, char const *, std::string, fstring_t<6> >
    data("test-ntuple.hdf5", "g1",
         {{"A", 2},
             "B",
             {"C", {2, 3}},
             {"D", 2},
             {"E", 2},
               make_column<fstring_t<6> >("F",
                 {},
                 PropertyList{H5P_DATASET_CREATE}(&H5Pset_deflate, 7u))},
         2);
  int i1data[] = { 1, 1, 2, 4, 3, 6, 5, 10, 7, 14, 11, 22, 13, 26, 17, 34, 23, 46};
  double d1data[] = { 1.01, 2.02, 3.03, 5.05, 7.07, 11.11, 13.13, 19.17, 23.23 };
  int i2data[] = { 0,   1,  2,  5,  6,  7,
                   10, 11, 12, 15, 16, 17,
                   20, 21, 22, 25, 26, 27,
                   30, 31, 32, 35, 36, 37,
                   40, 41, 42, 45, 46, 47,
                   50, 51, 52, 55, 56, 57,
                   60, 61, 62, 65, 66, 67,
                   70, 71, 72, 75, 76, 77,
                   80, 81, 82, 85, 86, 87 };
  char const * sdata[] =
    { "I wandered lonely as a cloud",
      "That floats on high o'er vales and hills,",
      "When all at once I saw a crowd,",
      "A host, of golden daffodils;",
      "Beside the lake, beneath the trees,",
      "Fluttering and dancing in the breeze.",
      "Continuous as the stars that shine",
      "And twinkle on the Milky Way,",
      "They stretched in never-ending line",
      "Along the margin of a bay:",
      "Ten thousand saw I at a glance,",
      "Tossing their heads in sprightly dance.",
      "The waves beside them danced, but they",
      "Out-did the sparkling waves in glee:",
      "A Poet could not but be gay,",
      "In such a jocund company:",
      "I gazed - and gazed - but little thought",
      "What wealth the show to me had brought:" };

  std::vector<std::string> const stringdata (std::cbegin(sdata),
                                             std::cend(sdata));

  fstring_t<6> factors[] = { "green", "red", "blue",
                             "green", "blue", "red",
                             "blue", "red", "green" };

  auto siter = stringdata.data();
  for (auto i = 0; i < 9; ++i) {
    data.insert(&i1data[i*2],
                d1data[i],
                &i2data[i*6],
                &sdata[i*2],
                siter + i * 2,
                factors[i]);
  }
}
