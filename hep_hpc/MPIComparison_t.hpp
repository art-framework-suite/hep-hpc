#ifndef HDFSTUDY_MPICOMPARISON_T_HPP
#define HDFSTUDY_MPICOMPARISON_T_HPP
////////////////////////////////////////////////////////////////////////
// MPIComparison.
//
// An enumeration class describing the different possible outcomes of an MPI comparison.
//
// Not all types of comparison operation may return all of these outcomes (a comparison of MPI groups may not return MPICompare::CONGRUENT, for example
#include "mpi.h"

namespace hep_hpc {
  namespace detail {
    enum class MPIComparison_t {
      IDENTICAL = MPI_IDENT,
        CONGRUENT = MPI_CONGRUENT,
        SIMILAR = MPI_SIMILAR,
        UNEQUAL = MPI_UNEQUAL
                  };
  }
  using detail::MPIComparison_t;
}
#endif /* HDFSTUDY_MPICOMPARISON_T_HPP */
