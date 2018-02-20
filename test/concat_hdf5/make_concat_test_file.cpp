////////////////////////////////////////////////////////////////////////
// make_concat_test_file -- Write a simple file for testing ntuple
// concatenation.
//
// For usage see make_concat_test_file -h.
//
////////////////////////////////////////////////////////////////////////

#include "hep_hpc/hdf5/Ntuple.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"
#include "hep_hpc/hdf5/make_column.hpp"
#include "hep_hpc/hdf5/make_ntuple.hpp"

#include <cstring>
#include <iostream>

using namespace hep_hpc;
using namespace hdf5;

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
    size_t n_rows() const { return n_rows_; }
    long row_offset() const { return row_offset_; }
    size_t chunk_size() const { return chunk_size_; }

    static void usage();

private:
    std::string output_ { "test.hdf5" };
    size_t n_rows_ { 1ull };
    long row_offset_ { 0l };
    size_t chunk_size_ { 128ull };
  };

  ProgramOptions::ProgramOptions(int argc, char **argv)
  {
    char *endptr = NULL;
    for (; ++argv, --argc;) {
      if (strcmp(*argv, "-o") == 0) {
        auto const tmp = std::strtol(argv[1], &endptr, 10);
        if (tmp == LONG_MIN || tmp == LONG_MAX || !*endptr == '\0') {
          throw
            ProgramOptionsException(std::string("Invalid input for offset: ") +
                                    argv[1], 2);
        }
        row_offset_ = tmp;
        --argc, ++argv;
      } else if (strcmp(*argv, "-n") == 0) {
        auto const tmp =  std::strtol(argv[1], &endptr, 10);
        if (tmp == LONG_MIN || tmp == LONG_MAX || !*endptr == '\0' || tmp < 0) {
          throw
            ProgramOptionsException(std::string("Invalid input for # rows: ") +
                                    argv[1], 2);
        }
        n_rows_ = tmp;
        --argc, ++argv;
      } else if (strcmp(*argv, "-c") == 0) {
        auto const tmp =  std::strtol(argv[1], &endptr, 10);
        if (tmp == LONG_MIN || tmp == LONG_MAX || !*endptr == '\0' || tmp < 0) {
          throw
            ProgramOptionsException(std::string("Invalid input for chunk size: ") +
                                    argv[1], 2);
        }
        chunk_size_ = tmp;
        --argc, ++argv;
      } else if (strcmp(*argv, "-h") == 0) {
        throw ProgramOptionsException("", 1);
      } else {
        break;
      }
    }
    if (argc == 1) {
      output_ = *argv;
    }
  }

  void
  ProgramOptions::usage()
  {
    std::cout << R"END(Usage: make_concat_test_file [-o <offset>] [-n <n-rows>] [<output-file>]
       make_concat_test_file -h

Options:

  -c <chunk-size>
     Chunk size for data (default 128).

  -h
     This help.

  -n <n-rows>
     Generate <n_rows> of data (default 1).

  -o <offset>
     Generate data as if for rows <offset> onward (default 0).


If not specified, <output-file> defaults to test.hdf5

)END";
  }
}

int main(int argc, char **argv)
{
  int status = 0;
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  try {
    ProgramOptions program_options(argc, argv);

    auto nt =
      make_ntuple({program_options.output(), "test_table", true},
                  make_column<long, 2>("data",
                    {2, 3},
                    program_options.chunk_size(),
                    {PropertyList(H5P_DATASET_CREATE)(H5Pset_deflate, 6)}));

    long const end_row = program_options.row_offset() + program_options.n_rows();

    for (long i_row = program_options.row_offset(); i_row < end_row; ++i_row) {
      std::array<long, 6> data {{ i_row * 100, i_row * 100 + 1, i_row * 100 + 2,
            i_row * 100 + 10, i_row * 100 + 11, i_row * 100 + 12 }};
      nt.insert(data.data());
    }
  }
  catch (ProgramOptionsException const & e) {
    std::cerr << e.what()
              << "\n";
    ProgramOptions::usage();
    status = e.status();
  }
  catch (std::exception const & e)  {
    std::cerr << "Exception while generating test file:\n"
              << e.what()
              << "\n";
      status = 1;
  }
  catch (...) {
    std::cerr << "Unknown exception while generating test file.\n";
    status = 1;
  }
  return status;
}
