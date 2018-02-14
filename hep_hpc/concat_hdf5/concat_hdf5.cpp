////////////////////////////////////////////////////////////////////////
// concat_hdf5
//
// Concatenate HDF5 files, combining compatible datasets (those with the
// same structure).
//
// This is functionally identical to the python program concat-hdf5.py,
// but is able to deal with compressed output under MPI conditions with
// collective writes (as of h5py 2.7.1, h5py erroneously elides
// collective writes for ranks with nothing to write, causing
// deadlocks).
//
// This program is certainly usable in a non-MPI environment, but
// scaling with MPI is highly desirable for large filesets.
//
// For usage see concat_h5py -h.
//
// 2017-02-13 CHG.
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/detail/config.hpp"
#include "hep_hpc/concat_hdf5/HDF5FileConcatenator.hpp"
#include "hep_hpc/concat_hdf5/maybe_report_rank.hpp"

#ifdef HEP_HPC_USE_MPI
#include "hep_hpc/MPI/MPIInstance.hpp"
#endif

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace hep_hpc;

namespace {
  class ProgramOptionsException : public std::runtime_error {
public:
    ProgramOptionsException(std::string const & what, int status)
      : std::runtime_error(what)
      , status_(status) {}

    int status() const { return status_; }

private:
    int status_;
  };

  class ProgramOptions {
public:
    ProgramOptions(int argc, char **argv);

    std::string const & output() const { return output_; }
    bool append() const { return append_; }
    bool overwrite() const { return overwrite_; }
    std::size_t mem_max() const { return mem_max_; }
    std::string const & filename_column() const { return filename_column_; }
    std::vector<std::string> const & only_groups() const { return only_groups_; }
    bool want_filters() const { return want_filters_; }
    bool want_collective_writes() const { return want_collective_writes_;}
    std::size_t verbosity() const { return verbosity_; }
    std::vector<std::string> const & inputs() const { return inputs_; }

private:
    std::string output_ { "test.hdf5" };
    bool append_ { false };
    bool overwrite_ { true };
    std::size_t mem_max_;
    std::string filename_column_;
    std::vector<std::string> only_groups_;
    bool want_filters_ { true };
    bool want_collective_writes_ { true };
    std::size_t verbosity_ { 3 };
    std::vector<std::string> inputs_;
  };

  ProgramOptions::ProgramOptions(int argc [[gnu::unused]],
                                 char **argv [[gnu::unused]])
    : inputs_(argv+1, argv+argc) // FIXME. Temporary.
  {
    // FIXME. Also need argument consistency checks e.g. with filters
    // and collective with MPI status, etc., etc.
  }

}

int main(int argc, char **argv)
{
  int status = 0;
#ifdef HEP_HPC_USE_MPI
  MPIInstance theMPI(argc,
                     argv,
                     MPIInstance::world_errors_return_tag);
#endif
  try {
    ProgramOptions program_options(argc, argv);
    HDF5FileConcatenator
      concatenator(program_options.output(),
                   program_options.append() ? H5F_ACC_RDWR : program_options.overwrite() ? H5F_ACC_TRUNC : H5F_ACC_EXCL,
                   program_options.mem_max(),
                   program_options.filename_column(),
                   program_options.only_groups(),
                   program_options.want_filters(),
                   program_options.want_collective_writes(),
                   program_options.verbosity());
    status = concatenator.concatFiles(program_options.inputs());
  }
  catch (ProgramOptionsException const & e) {
    std::cerr << maybe_report_rank
              << e.what()
              << "\n";
    status = e.status();
  }
  catch (std::exception const & e)  {
    std::cerr << maybe_report_rank
              << "Exception while concatenating files:\n"
              << e.what()
              << "\n";
      status = 1;
  }
  catch (...) {
    std::cerr << maybe_report_rank
              << "Unknown exception while concatenating files.\n";
    status = 1;
  }
#ifdef HEP_HPC_USE_MPI
  if (status != 0) {
    theMPI.abort(status);
  }
#endif
  return status;
}
