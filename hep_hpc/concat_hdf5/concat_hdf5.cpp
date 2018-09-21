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
#include "hep_hpc/concat_hdf5/FilenameColumnInfo.hpp"
#include "hep_hpc/concat_hdf5/HDF5FileConcatenator.hpp"
#include "hep_hpc/concat_hdf5/maybe_report_rank.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#ifdef HEP_HPC_USE_MPI
#include "hep_hpc/MPI/MPIInstance.hpp"
#include "hep_hpc/MPI/MPICommunicator.hpp"
#endif

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace hep_hpc;

namespace {
  // File-scope parameters.
  int n_ranks;
  int my_rank;

  // Produce an unbundled copy of the argument list.
  std::vector<std::string>
  unbundle_args(int argc, char **argv)
  {
    // Regular expression to match --arg=[arg1[,arg2]+].
    static std::regex
      long_equals_arg("(--[-_[:alnum:]]+)=((?:(?:[^,]+),)*?(?:[^,]+))?");
    std::cmatch lea_match;
    std::vector<std::string> result;
    result.reserve(argc);
    for (; ++argv, --argc;) {
      if ((*argv)[0] == '-' && (*argv)[1] != '-' && strlen(*argv) > 2) {
        // Deal with bundled short option arguments (-).
        for (auto i = *argv + 1; *i != '\0'; ++i) {
          result.emplace_back(std::string("-") + *i);
        }
      } else if ((*argv)[0] == '+' && strlen(*argv) > 2) {
        // Deal with bundled short option arguments (+).
        for (auto i = *argv + 1; *i != '\0'; ++i) {
          result.emplace_back(std::string("+") + *i);
        }
      } else if (std::regex_match(*argv, lea_match, long_equals_arg)) {
        // Deal with --arg=... arguments
        result.emplace_back(lea_match[1].str()); // Argumemt name.
        auto const & subargs = lea_match[2].str();
        if (subargs.empty()) { // Single empty arg.
          result.emplace_back();
        } else {
          std::string::size_type pos = 0, last_pos = std::string::npos;
          while ((pos = subargs.find(',', last_pos + 1)) != std::string::npos) {
            result.emplace_back(subargs.substr(last_pos + 1, pos - last_pos - 1));
            last_pos = pos;
          }
          // Last argument.
          result.emplace_back(subargs.substr(last_pos +1));
        }
        if (argc == 1 ||
            !(argv[1][0] == '-' || argv[1][0] == '+')) { // Look ahead.
          // Last option argument before non-option arguments: armor!
          result.emplace_back("--");
        }
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
  void throw_poe(std::string msg,
                 int status,
                 std::string const & extra_msg = {})
  {
    if (!msg.empty()) {
      if (!extra_msg.empty()) {
        msg += ": " + extra_msg;
      }
    } else {
      msg += extra_msg;
    }
    throw ProgramOptionsException(msg, status);
  }

  [[noreturn]]
  void throw_bad_argument(std::string const & opt,
                          std::string const & arg,
                          int const status,
                          std::string const & extra_msg = {})
  {
    throw_poe(std::string("Bad argument \"") +
              arg + "\" to option \"" + opt + ".\"",
              status,
              extra_msg);
  }

  [[noreturn]]
  void
  throw_bad_sub_args(std::string const & opt,
                     int const status,
                     std::string const & extra_msg = {})
  {
    throw_poe(std::string("Bad number of arguments to option \"") +
              opt + ".\"",
              status, extra_msg);
  }

  template<typename IN_ITER>
  void
  coerce_n_sub_args_(size_t const n_sub_args,
                     IN_ITER const iarg,
                     IN_ITER & next_arg_iter)
  {
    next_arg_iter = detail::copy_advance(iarg, 1 + n_sub_args);
  }

  template <typename STRING_COLL>
  void
  read_file_list(std::string const & list_file_name,
                 STRING_COLL & coll)
  {
    std::ifstream file_list(list_file_name);
    std::string read_file_name;
    while (std::getline(file_list, read_file_name)) {
      if (read_file_name.empty() || read_file_name.front() == '#') {
        continue;
      }
      coll.push_back(read_file_name);
    }
  }

  class ProgramOptions {
public:
    ProgramOptions(int argc, char **argv);

    std::string const & output() const { return output_; }
    bool append() const { return append_; }
    bool overwrite() const { return overwrite_; }
    long long max_rows() const { return max_rows_; }
    std::size_t mem_max_bytes() const { return mem_max_bytes_; }
    FilenameColumnInfo const & filename_column_info() const { return filename_column_info_; }
    FilenameColumnInfo & filename_column_info() { return filename_column_info_; }
    std::vector<std::regex> const & only_groups() const { return only_groups_; }
    bool want_filters() const { return want_filters_; }
    bool force_compression() const { return force_compression_; }
    bool want_collective_writes() const { return want_collective_writes_;}
    bool want_flush_per_dataset() const { return want_flush_per_dataset_; }
    bool want_mpi_io() const { return want_mpi_io_; }
    std::size_t verbosity() const { return verbosity_; }
    std::vector<std::string> const & inputs() const { return inputs_; }

    static void usage();

private:
    static std::string const DEFAULT_FILENAME;
    static bool const DEFAULT_APPEND;
    static bool const DEFAULT_OVERWRITE;
    static long long const DEFAULT_MAX_ROWS;
    static std::size_t const DEFAULT_MEM_MAX;
    static bool const DEFAULT_WANT_FILTERS;
    static bool const DEFAULT_FORCE_COMPRESSION;
    static bool const DEFAULT_WANT_COLLECTIVE_WRITES;
    static bool const DEFAULT_WANT_FLUSH_PER_DATASET;
    static bool const DEFAULT_WANT_MPI_IO;
    static int const DEFAULT_VERBOSITY;

    std::string output_ { DEFAULT_FILENAME };
    bool append_ { DEFAULT_APPEND };
    bool overwrite_ { DEFAULT_OVERWRITE };
    long long max_rows_ { DEFAULT_MAX_ROWS };
    std::size_t mem_max_bytes_ { DEFAULT_MEM_MAX * 1024 * 1024 };
    FilenameColumnInfo filename_column_info_;
    std::vector<std::regex> only_groups_;
    bool want_filters_ { DEFAULT_WANT_FILTERS };
    bool force_compression_ { DEFAULT_FORCE_COMPRESSION };
    bool want_collective_writes_ { DEFAULT_WANT_COLLECTIVE_WRITES };
    bool want_flush_per_dataset_ { DEFAULT_WANT_FLUSH_PER_DATASET };
    bool want_mpi_io_ { DEFAULT_WANT_MPI_IO };
    int verbosity_ { DEFAULT_VERBOSITY };
    std::vector<std::string> inputs_;
  };

  ProgramOptions::ProgramOptions(int argc, char **argv)
  {
    using std::to_string;
    std::vector<std::string> args(unbundle_args(argc, argv));
    std::size_t idx {0ull};
    auto const eargs = args.end();
    auto iarg = args.begin(), next_arg_iter = iarg;
    for (; iarg != eargs; iarg = next_arg_iter) {
      auto & arg = *iarg;
      if (arg[0] != '-' && arg[0] != '+') {
        // Finished processing options (processing for individual
        // options should progress iarg after dealing with arguments).
        break;
      } else if (arg == "--") {
        // Explicitly finished processing options.
        ++iarg;
        break;
      }
      // Count sub-arguments (greedy).
      next_arg_iter = iarg + 1;
      while (next_arg_iter != eargs &&
             (*next_arg_iter)[0] != '-' &&
             (*next_arg_iter)[0] != '+') {
        ++next_arg_iter;
      }
      auto n_sub_args = std::distance(iarg + 1, next_arg_iter);
      // Allow adjustment of arg pointers.
      auto coerce_n_sub_args =
        [iarg, &next_arg_iter, &n_sub_args](size_t set_sub_args) {
        coerce_n_sub_args_(set_sub_args, iarg, next_arg_iter);
        n_sub_args = set_sub_args;
      };
      if (arg[1] == '-') { // Long options.
        if (arg == "--append") {
          arg = "-a"; // Short option alias.
        } else if (arg == "--collective-writes") {
          arg = "-C"; // Short option alias.
        } else if (arg == "--no-collective-writes") {
          arg = "+C"; // Short option alias.
        } else if (arg == "--input-file-list") {
          arg = "-I"; // Short option alias.
        } else if (arg == "--filename-column") {
          if (n_sub_args != 1 && n_sub_args < 3) {
            throw_bad_sub_args(*iarg,
                               2,
                               std::string("expected 1 or 3+ sub-arguments, got ") +
                               to_string(n_sub_args));
          }
          if (n_sub_args == 1) {
            // Only filename column.
            filename_column_info_ = FilenameColumnInfo(*(iarg + 1));
          } else {
            try {
              // Filename column, regex and replacememt string.
              filename_column_info_ =
                FilenameColumnInfo(*(iarg + 1), std::regex(*(iarg + 2)), *(iarg + 3));
            }
            catch (std::regex_error const & e) {
              throw_bad_argument(arg, *(iarg + 1), 2, e.what());
            }
            // Any group regexes.
            std::vector<std::regex> group_regexes;
            for (auto isubarg = iarg + 4;
                 isubarg != next_arg_iter;
                 ++isubarg) {
              try {
                group_regexes.emplace_back(*isubarg);
              }
              catch (std::regex_error const & e) {
                throw_bad_argument(arg, *isubarg, 2, e.what());
              }
            }
            filename_column_info_.set_group_regexes(std::move(group_regexes));
          }
          continue;
        } else if (arg == "--force-compression") {
          coerce_n_sub_args(0);
          force_compression_ = true;
          continue;
        } else if (arg == "--help") {
          arg = "-h"; // Short option alias.
        } else if (arg == "--max-rows") {
          arg = "-n"; // Short option alias.
        } else if (arg == "--mem-max") {
          coerce_n_sub_args(1);
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
          coerce_n_sub_args(1);
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
        } else if (arg == "--mpi-io") {
          coerce_n_sub_args(0);
          want_mpi_io_ = true;
          continue;
        } else if (arg == "--only-groups") {
          if (n_sub_args == 0) {
            throw_bad_sub_args(*iarg,
                               2,
                               "expected at least one sub-argument");
          }
          for (auto isubarg = detail::copy_advance(iarg, 1);
               isubarg != next_arg_iter;
               ++isubarg) {
            try {
              only_groups_.emplace_back(*isubarg);
            }
            catch (std::regex_error const & e) {
              throw_bad_argument(arg, *isubarg, 2, e.what());
            }
          }
          continue;
        } else if (arg == "--output") {
          arg = "-o"; // Short option alias.
        } else if (arg == "--overwrite") {
          arg = "-f"; // Short option alias.
        } else if (arg == "--verbose") {
          arg = "-v"; // Short option alias.
        } else if (arg == "--verbosity") {
          coerce_n_sub_args(1);
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
        coerce_n_sub_args(0);
        append_ = true;
        break;
      case 'C':
        coerce_n_sub_args(0);
        want_collective_writes_ = (arg[0] == '-') ? true : false;
        break;
      case 'F':
        coerce_n_sub_args(0);
        want_filters_ = (arg[0] == '-') ? true : false;
        break;
      case 'I':
        coerce_n_sub_args(1);
        try {
          read_file_list(*++iarg, inputs_);
        } catch (std::exception const & e) {
          throw_bad_argument(arg, *iarg, 2, e.what());
        } catch (...) {
          throw_bad_argument(arg, *iarg, 2);
        }
        break;
      case 'f':
        coerce_n_sub_args(0);
        overwrite_ = true;
        break;
      case 'h':
        FALLTHROUGH;
      case '?':
        throw ProgramOptionsException(*iarg, 1);
        break;
      case 'n':
      {
        coerce_n_sub_args(1);
        try {
          max_rows_ = std::stoll(*++iarg, &idx);
        }
        catch (...) {
          throw_bad_argument(arg, *iarg, 2);
        }
        if (idx != iarg->size()) {
          throw_bad_argument(arg, *iarg, 2);
        }
        continue;
      }
      case 'o':
        coerce_n_sub_args(1);
        output_ = *(iarg + 1);
        break;
      case 'v':
        coerce_n_sub_args(0);
        verbosity_ += (arg[0] == '-') ? 1 : -1;
        break;
      default:
        throw
          ProgramOptionsException(std::string("Unrecognized option: ") +
                                  arg, 1);
      }
    }

    if (force_compression_ && !want_filters_) {
      std::cerr << "WARNING: Forced compression requires filters to be enabled in the"
                << "         output.";
        want_filters_ = true;
    }

    if (want_collective_writes_) {
#if ! (H5_VERS_MAJOR > 1 ||                                             \
       (H5_VERS_MAJOR == 1 && H5_VERS_MINOR > 10) ||                    \
       (H5_VERS_MAJOR == 1 && H5_VERS_MINOR == 10 && H5_VERS_RELEASE >= 3))
      if (n_ranks == 1) {
        std::cerr << "WARNING: Collective writes require > 1 MPI processes for HDF5 version "
                  << H5_VER_INFO << ".\n";
        want_collective_writes_ = false;
      }
#endif
#if ! (H5_VERS_MAJOR > 1 ||                                             \
       (H5_VERS_MAJOR == 1 && H5_VERS_MINOR > 10) ||                    \
       (H5_VERS_MAJOR == 1 && H5_VERS_MINOR == 10 && H5_VERS_RELEASE >= 2))
      else if (want_filters_) {
        if (my_rank == 0) {
          std::cerr << "WARNING: Collective writes not supported with filters for HDF5 version "
                    << H5_VERS_INFO << ".\n";
          std::cerr << "         Require HDF5 version >= 1.10.2.\n"
                    << "         Deactivating output filters.\n";
        }
        want_filters_ = false;
      }
#endif
    } else if (want_filters_ && (n_ranks > 1 || want_mpi_io_)) {
      if (my_rank == 0) {
        std::cerr << "WARNING: Output filters require collective writes under MPI I/O.\n"
                  << "         Deactivating output filters.\n";
      }
      want_filters_ = false;
    }

    if (append_ and overwrite_) {
      throw ProgramOptionsException("Append and overwrite are mutually exclusive.", 2);
    }

    if (output_.empty()) {
      throw ProgramOptionsException("Empty output file name", 2);
    }

    inputs_.insert(inputs_.end(), iarg, eargs);
  }

  void
  ProgramOptions::usage()
  {
    std::cerr << R"END(Usage: concat_hdf5 [<OPTIONS>] [--] [<input-file>+]
       concat_hdf5 --help|-h|-?

Concatenate tabular HDF5 files, combining compatible datasets into one.


OPTIONS

  --append
  -a

    Append to an existing output file (default )END"
              << std::boolalpha << DEFAULT_APPEND << R"END().
    Mutually exclusive with --overwrite.

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

    NOTES

    * <regex> is a substring match expression following the ECMAScript
      specification. Use ^ and $ as appropriate to anchor the beginning
      and end of your expression.

    * The replacement format string should follow the ECMAScript
      specifications for backreferences (e.g. $<num> for back-reference
      <num>) as described in documentation for the C++ Standard Library
      function std::regex_replace().

    * <group-regex> arguments, if specified, are whole-string match
      expressions following the ECMAScript specification.

  -I <list-file>
  --input-file-list <list-file>

    Obtain the list of input files from <list-file> rather than from the
    command line. Each line should contain one file name. Empty lines
    and those starting with # will be skipped. Inline comments are *not*
    permitted and leading spaces will be considered part of the file
    name.

  --flush-per-dataset

    Ensure that HDF information is flushed to the file after every
    dataset is written. This causes a performance penalty and is only
    useful for clarity while debugging or studying performance
    bottlenecks (default )END"
              << std::boolalpha << DEFAULT_WANT_FLUSH_PER_DATASET
              << R"END().

  --force-compression

    Force compression (deflate, level 6) on columns with none set
    set (default )END"
              << std::boolalpha << DEFAULT_FORCE_COMPRESSION
              << R"END(). Requires filters (--with-filters) and
    (if invoking multiple MPI proceses) collective writes
    (--collective-writes) to be selected.

  --help
  -h
  -?

    This help.

  --max-rows <max>
  -n <max>

    Write no more than <max> rows per dataset. A value of -1 (the
    default) indicates to write all rows found.

  --mem-max <buffer-size-MiB>

    The maximum memory available for the I/O buffer, in MiB (default )END"
              << DEFAULT_MEM_MAX << R"END( MiB).

  --mem-max-bytes <buffer-size-B>

    The maximum memory available for the I/O buffer, in B (default )END"
              << DEFAULT_MEM_MAX * 1024 * 1024 << R"END( B).

  --mpi-io

    Select MPI I/O even in the case when nranks == 1 (default )END"
              << std::boolalpha << DEFAULT_WANT_MPI_IO << R"END().

  --only-groups <regex>+

    Handle only groups matching these ECMAScript whole-string match
    expressions, and the datasets within them.

  --output <output-file>
  -o <output-file>

    Specify the name of the HDF5 output file (default ")END"
              << DEFAULT_FILENAME << R"END(").

  --overwrite
  -f

    Overwrite an existing output file of the same name (default )END"
              << DEFAULT_OVERWRITE << R"END().
    Mutually exclusive with --append.

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
    Output filters require a modern  HDF5 library and collective writes
    with MPI I/O (see --collective-writes above, and notes below).


NOTES

* `+' denotes a repeatable argument.

* Short options may be bundled e.g. `-af' => `-a -f' and `+CF' =>
  `+C +F'.

* Long options may be specified as --arg arg1 arg2 ... or
  --arg=arg1,arg2... However, if your arguments include commas you
  should use the first form. If your arguments contain spaces you
  should use quotation marks or escapes (\) to hide them from the shell.
  If you specify a space-separated long option argument set as your
  final option argument, you should finish with `--'  to
  distinguish arguments to the long option from non-option arguments.
  `--long-arg=' shall be equivalent to `--long-arg ""'.

* concat_hdf5 works best with files created with hep_hpc::hdf5::Ntuple.

* If used with a modern-enough HDF5, it is capable of concatenating
  filtered (e.g. compressed) data into files while utilizing parallel
  I/O with MPI. With older HDF5 (<=1.10), either deactivate MPI or
  deactivate filters.

* Ordinarily, parallel MPI I/O is only selected if nranks > 1 because we
  are incapable in the general case of detecting the difference between
  invoking the program in a non-MPI environment and invoking it in an
  MPI environment with nranks == 1. On some HPC filesystems it is
  necessary to use MPI I/O even if nranks == 1: in this case, use the
  --mpi-io option. Note that for older versions of HDF5 this may be
  incompatible with filter use in the output.
)END";
  }

  std::string const ProgramOptions::DEFAULT_FILENAME = "test.hdf5";
  bool const ProgramOptions::DEFAULT_APPEND = false;
  bool const ProgramOptions::DEFAULT_OVERWRITE = true;
  long long const ProgramOptions::DEFAULT_MAX_ROWS = -1ll;
  std::size_t const ProgramOptions::DEFAULT_MEM_MAX = 100ull;
  bool const ProgramOptions::DEFAULT_WANT_FILTERS = true;
  bool const ProgramOptions::DEFAULT_FORCE_COMPRESSION = false;
  bool const ProgramOptions::DEFAULT_WANT_COLLECTIVE_WRITES = true;
  bool const ProgramOptions::DEFAULT_WANT_FLUSH_PER_DATASET = false;
  bool const ProgramOptions::DEFAULT_WANT_MPI_IO = false;
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
  bool want_abort [[gnu::unused]] = false;
#endif
  // Make sure we throw exceptions on HDF5 call errors.
  hdf5::ErrorController::setErrorHandler(hdf5::ErrorMode::EXCEPTION);

  // Do the work with the appropriate protections.
  try {
    ProgramOptions program_options(argc, argv);
    HDF5FileConcatenator
      concatenator(program_options.output(),
                   program_options.append() ?
                   H5F_ACC_RDWR :
                   program_options.overwrite() ? H5F_ACC_TRUNC : H5F_ACC_EXCL,
                   program_options.max_rows(),
                   program_options.mem_max_bytes(),
                   std::move(program_options.filename_column_info()),
                   program_options.only_groups(),
                   program_options.want_filters(),
                   program_options.force_compression(),
                   program_options.want_collective_writes(),
                   program_options.want_flush_per_dataset(),
                   program_options.want_mpi_io(),
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
