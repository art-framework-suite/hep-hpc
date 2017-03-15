#include "hep_hpc/Ntuple.hpp"

using namespace hep_hpc;

int main()
{
  using namespace std::string_literals;
  Ntuple<int, double>("test-ntuple.hdf5", "g1", { "I1"s, "D1"s });
}
