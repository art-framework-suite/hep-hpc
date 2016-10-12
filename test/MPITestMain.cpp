#include "hep_hpc/MPIInstance.hpp"

#include "gtest/gtest.h"

#include <iostream>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::hep_hpc::MPIInstance theMPI(argc, argv, ::hep_hpc::MPIInstance::world_errors_return_tag);
  return RUN_ALL_TESTS();
}
