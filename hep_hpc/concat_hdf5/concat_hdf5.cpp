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
// For usage see concat_hdf5 -h.
//
// 2017-02-13 CHG.
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/detail/config.hpp"

#include "hep_hpc/Utilities/detail/compiler_macros.hpp"
#include "hep_hpc/concat_hdf5/HDF5FileConcatenator.hpp"
#include "hep_hpc/concat_hdf5/maybe_report_rank.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#ifdef HEP_HPC_USE_MPI
#include "hep_hpc/MPI/MPIInstance.hpp"
#include "hep_hpc/MPI/MPICommunicator.hpp"
#endif

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace hep_hpc;

namespace {
  // File-scope parameters.
  int n_ranks;
  int my_rank;

  // Produced an unbundled copy of the argument list.
  std::vector<std::string>
  unbundle_args(int argc, char **argv)
  {
    std::vector<std::string> result;
    result.reserve(argc);
    for (; ++argv, --argc;) {
      if ((*argv)[0] == '-' && (*argv)[1] != '-' && strlen(*argv) > 2) {
        for (auto i = *argv + 1; *i != '\0'; ++i) {
          result.emplace_back(std::string("-") + *i);
        }
// FIXME: complete this implementation. Don't forget to insert `--' if
//        we're expanding the last argument.
//
//    } else if () { // Deal with --arg=... arguments
      } else {
        result.emplace_back(*argv);
      }
    }
    return result;
  }

  class ProgramOptionsException : public std::runtime_error {
public:
    ProgramOptionsException(std::string const & what, int status)
      : std::runtime_error(what)
      , status_(status) {}

    int status() const { return status_; }

private:
    int status_;
  };

  [[noreturn]]
  void throw_bad_argument(std::string const & opt,
                          std::string const & arg,
                          int status)
  {
    throw
      ProgramOptionsException(std::string("Bad argument \"") +
                              arg + "\" to option \"" + opt + ".\"",
                              status);
  }

  class ProgramOptions {
public:
    ProgramOptions(int argc, char **argv);

    std::string const & output() const { return output_; }
    bool append() const { return append_; }
    bool overwrite() const { return overwrite_; }
    std::size_t mem_max_bytes() const { return mem_max_bytes_; }
    std::string const & filename_column() const { return filename_column_; }
    std::vector<std::string> const & only_groups() const { return only_groups_; }
    bool want_filters() const { return want_filters_; }
    bool want_collective_writes() const { return want_collective_writes_;}
    std::size_t verbosity() const { return verbosity_; }
    std::vector<std::string> const & inputs() const { return inputs_; }

    static void usage();

private:
    static std::string const DEFAULT_FILENAME;
    static bool const DEFAULT_APPEND;
    static bool const DEFAULT_OVERWRITE;
    static std::size_t const DEFAULT_MEM_MAX;
    static bool const DEFAULT_WANT_FILTERS;
    static bool const DEFAULT_WANT_COLLECTIVE_WRITES;
    static int const DEFAULT_VERBOSITY;

    std::string output_ { DEFAULT_FILENAME };
    bool append_ { DEFAULT_APPEND };
    bool overwrite_ { DEFAULT_OVERWRITE };
    std::size_t mem_max_bytes_ { DEFAULT_MEM_MAX * 1024 * 1024 };
    std::string filename_column_;
    std::vector<std::string> only_groups_;
    bool want_filters_ { DEFAULT_WANT_FILTERS };
    bool want_collective_writes_ { DEFAULT_WANT_COLLECTIVE_WRITES };
    int verbosity_ { DEFAULT_VERBOSITY };
    std::vector<std::string> inputs_;
  };

  ProgramOptions::ProgramOptions(int argc, char **argv)
  {
    std::vector<std::string> args(unbundle_args(argc, argv));
    std::size_t idx {0ull};
    auto const eargs = args.end();
    auto iarg = args.begin();
    for (; iarg != eargs; ++iarg) {
      auto arg = *iarg;
      if (arg[0] != '-' && arg[0] != '+') {
        // Finished processing options (processing for individual
        // options should progress iarg after dealing with arguments).
        break;
      } else if (arg == "--") {
        // Explicitly finished processing options.
        ++iarg;
        break;
      }
      if (arg[1] == '-') { // Long options.
        if (arg == "--append") {
          arg = "-a"; // Short option alias.
        } else if (arg == "--collective-writes") {
          arg = "-C"; // Short option alias.
        } else if (arg == "--no-collective-writes") {
          arg = "+C"; // Short option alias.
        } else if (arg == "--filename-column") {
          continue;
        } else if (arg == "--help") {
          arg = "-h"; // Short option alias.
        } else if (arg == "--mem-max") {
          try {
            mem_max_bytes_ = std::stoull(*++iarg, &idx) * 1024 * 1024;
          }
          catch (...) {
            throw_bad_argument(arg, *iarg, 2);
          }
          if (idx != iarg->size()) {
            throw_bad_argument(arg, *iarg, 2);
          }
          continue;
        } else if (arg == "--mem-max-bytes") {
          try {
            mem_max_bytes_ = std::stoull(*++iarg, &idx);
          }
          catch (...) {
            throw_bad_argument(arg, *iarg, 2);
          }
          if (idx != iarg->size()) {
            throw_bad_argument(arg, *iarg, 2);
          }
          continue;
        } else if (arg == "--only-groups") {
          continue;
        } else if (arg == "--output") {
          arg = "-o"; // Short option alias.
        } else if (arg == "--overwrite") {
          arg = "-f"; // Short option alias.
        } else if (arg == "--verbose") {
          arg = "-v"; // Short option alias.
        } else if (arg == "--verbosity") {
          try {
            verbosity_ = std::stoi(*++iarg, &idx);
          }
          catch (...) {
            throw_bad_argument(arg, *iarg, 2);
          }
          if (idx != iarg->size()) {
            throw_bad_argument(arg, *iarg, 2);
          }
          continue;
        } else if (arg == "--with-filters") {
          arg = "-F"; // Short option alias.
        } else if (arg == "--without-filters") {
          arg = "+F"; // Short option alias.
        } else {
          throw
            ProgramOptionsException(std::string("Unrecognized long option: ") +
                                    arg, 1);
        }
      }
      // Process short options (including aliases for long options).
      switch (arg[1]) {
      case 'a':
        append_ = true;
        break;
      case 'C':
        want_collective_writes_ = (arg[0] = '-') ? true : false;
        break;
      case 'F':
        want_filters_ = (arg[0] = '-') ? true : false;
        break;
      case 'f':
        overwrite_ = true;
        break;
      case 'h':
        FALLTHROUGH;
      case '?':
        throw ProgramOptionsException(*iarg, 1);
        break;
      case 'o':
        output_ = *++iarg;
        break;
      case 'v':
        verbosity_ += (arg[0] = '-') ? 1 : -1;
        break;
      default:
        throw
          ProgramOptionsException(std::string("Unrecognized option: ") +
                                  arg, 1);
      }
    }

    // FIXME. Also need argument consistency checks e.g. with filters
    // and collective with MPI status, etc., etc.
    inputs_.insert(inputs_.end(), iarg, eargs);
  }

  void
  ProgramOptions::usage()
  {
    std::cerr << R"END(Usage: concat_hdf5 [<OPTIONS>] [--] <input-file>+
       concat_hdf5 --help|-h|-?

Concatenate tabular HDF5 files, combining compatible datasets into one.

OPTIONS

  --append
  -a

    Append to an existing output file (default )END"
              << std::boolalpha << DEFAULT_APPEND << R"END().

  --collective-writes
  -C
  --no-collective-writes
  +C

    When invoked with multiple MPI processes, use collective read/write
    operations (default )END"
              << std::boolalpha
              << DEFAULT_WANT_COLLECTIVE_WRITES
              << R"END(). This is necessary (along
    with a modern HDF5 (see notes below) if (e.g.) compression is
    desired in output datasets.

  --filename-column <column-name> [<regex> <replacement-expression>
                                   [<group-regex>+]]

    Add a column (dataset) whose value is extracted from the full input
    path by the provided regex replacement to all groups or just those
    matching specified regexes. The colum will have as many rows as the
    largest dataset in that group, and the value will match the
    extracted value from the path of the input file who dataset(s)
    triggered the resize. This column is only useful for tabular data
    where each dataset in a given group has the same number of rows.

  --help
  -h
  -?

    This help.

  --mem-max <buffer-size-MiB>

    The maximum memory available for the I/O buffer, in MiB (default )END"
              << DEFAULT_MEM_MAX << R"END( MiB).

  --mem-max-bytes <buffer-size-B>

    The maximum memory available for the I/O buffer, in B (default )END"
              << DEFAULT_MEM_MAX * 1024 * 1024 << R"END( B).

  --only-groups <regex>+

    Handle only groups matching these regexes, and the datasets within
    them.

  --output <output-file>
  -o <output-file>

    Specify the name of the HDF5 output file (default ")END"
              << DEFAULT_FILENAME << R"END(").

  --overwrite
  -f

    Overwrite an existing output file of the same name (default )END"
              << DEFAULT_OVERWRITE << R"END().
    Mutually-incompatible with --append.

  --verbose+
  [-+]v+
  --verbosity <#>

    Specify the verbosity of messages, either by repeating --verbose or
    [-+]v options, or explicitly with --verbosity <#> (default )END"
              << DEFAULT_VERBOSITY << R"END().

  --with-filters
  -F
  --without-filters
  +F

    Specify whether filters should be propagated from the first input
    file to the output (default )END"
              << std::boolalpha << DEFAULT_WANT_FILTERS << R"END().

NOTES

* `+' denotes a repeatable argument.

* Short options may be bundled.

* Long options may be specified as --arg arg1 arg2 ... or
  --arg=arg1,arg2... However, if your arguments include commas you
  should use the first form. If your arguments contain spaces you
  should use quotation marks to hide them from the shell.

* concat_hdf5 works best with files created with hep_hpc::hdf5::Ntuple.

* If used with a modern-enough HDF5, it is capable of concatenating
  compressed data into files while utilizing parallel I/O with MPI. With
  older HDF5 (<=1.10), either deactivate MPI or deactivate filters.

)END";
  }

  std::string const ProgramOptions::DEFAULT_FILENAME = "test.hdf5";
  bool const ProgramOptions::DEFAULT_APPEND = false;
  bool const ProgramOptions::DEFAULT_OVERWRITE = true;
  std::size_t const ProgramOptions::DEFAULT_MEM_MAX = 100ull;
  bool const ProgramOptions::DEFAULT_WANT_FILTERS = true;
  bool const ProgramOptions::DEFAULT_WANT_COLLECTIVE_WRITES = true;
  int const ProgramOptions::DEFAULT_VERBOSITY = 0;

}



int main(int argc, char **argv)
{
  int status = 0;
#ifdef HEP_HPC_USE_MPI
  MPIInstance theMPI(argc,
                     argv,
                     MPIInstance::world_errors_return_tag);
  hep_hpc::MPICommunicator world;
  n_ranks = world.size();
  my_rank = world.rank();
  bool want_abort = (n_ranks > 1);
#else
  n_ranks = 1;
  my_rank = 0;
  bool want_abort = false;
#endif
  // Make sure we throw exceptions on HDF5 call errors.
//  hdf5::ErrorController::setErrorHandler(hdf5::ErrorMode::EXCEPTION);

  // Do the work with the appropriate protections.
  try {
    ProgramOptions program_options(argc, argv);
    HDF5FileConcatenator
      concatenator(program_options.output(),
                   program_options.append() ?
                   H5F_ACC_RDWR :
                   program_options.overwrite() ? H5F_ACC_TRUNC : H5F_ACC_EXCL,
                   program_options.mem_max_bytes(),
                   program_options.filename_column(),
                   program_options.only_groups(),
                   program_options.want_filters(),
                   program_options.want_collective_writes(),
                   program_options.verbosity());
    status = concatenator.concatFiles(program_options.inputs());
  }
  catch (ProgramOptionsException const & e) {
    if (my_rank == 0) {
      std::cerr << maybe_report_rank
                << e.what()
                << "\n";
      ProgramOptions::usage();
    }
    status = e.status();
    want_abort = false; // Let everyone finish normally.
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
  if (status != 0 && want_abort) {
    theMPI.abort(status);
  }
#endif
  return status;
}
