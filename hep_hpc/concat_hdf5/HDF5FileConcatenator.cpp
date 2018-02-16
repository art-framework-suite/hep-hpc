#include "hep_hpc/concat_hdf5/HDF5FileConcatenator.hpp"

#include "hep_hpc/detail/config.hpp"
#ifdef HEP_HPC_USE_MPI
#include "hep_hpc/MPI/MPICommunicator.hpp"
#endif

#include "hep_hpc/concat_hdf5/maybe_report_rank.hpp"
#include "hep_hpc/hdf5/Dataset.hpp"
#include "hep_hpc/hdf5/Group.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/ResourceStrategy.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

extern "C" {
#include "sys/stat.h"
#include "H5FDmpio.h"
#include "H5Spublic.h"
}

#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>

using namespace hep_hpc::hdf5;

namespace {
  // File-scope parameters.
  int verbosity;
  int n_ranks;
  int my_rank;

  bool file_exists(std::string file_name)
  {
    struct stat buffer;
    return (stat(file_name.c_str(), &buffer) == 0);
  }

  void report(int level, std::string const & msg)
  {
    using std::to_string;
    if (verbosity < level) {
      return;
    }
    std::string level_txt;
    switch (level) {
    case -2:
      level_txt = "ERROR";
      break;
    case -1:
      level_txt = "WARNING";
      break;
    case 0:
      level_txt = "INFO";
      break;
    default:
      level_txt = std::string("DEBUG(") + to_string(level) + ')';
    }
    std::ostream & os = (level > -1) ? std::cout : std::cerr;
    os << maybe_report_rank
       << level_txt
       << ": "
       << msg
       << std::endl;
  }

  PropertyList
  maybe_collective_access()
  {
    PropertyList file_access_properties(H5P_FILE_ACCESS);
#ifdef HEP_HPC_USE_MPI
    report(1, "Setting access properties to use MPI I/O.");
    (void) ErrorController::call(&H5Pset_fapl_mpio,
                                 file_access_properties,
                                 MPI_COMM_WORLD,
                                 MPI_INFO_NULL);
#endif
    return file_access_properties;
  }

  File
  open_output_file(std::string file_name,
                   unsigned int file_mode)
  {
    // We want a slightly different semantic for "append" (H5F_ACC_RDWR)
    // than that provided by HDF5: if it exists, open it read/write; if it
    // does not, create it.
    if (file_mode == H5F_ACC_RDWR &&  ! file_exists(file_name)) {
      file_mode = H5F_ACC_EXCL;
    }
    File output_file(file_name, file_mode, {}, maybe_collective_access());
    return output_file;
  }

  Dataspace
  output_dataspace(Dataspace const & in_dataspace)
  {
    Dataspace result = in_dataspace;
    auto const ndims = ErrorController::call(&H5Sget_simple_extent_ndims, result);
    std::vector<hsize_t> shape(ndims), maxshape(ndims);
    (void) ErrorController::call(&H5Sget_simple_extent_dims,
                                 result,
                                 shape.data(),
                                 maxshape.data());
    maxshape.front() = H5S_UNLIMITED;
    (void) ErrorController::call(&H5Sset_extent_simple,
                                 result,
                                 ndims,
                                 shape.data(),
                                 maxshape.data());
    return result;
  }

  struct NumerologyInfo {
    std::size_t rows_to_write_this_iteration {0ull};
    std::size_t rows_to_write_this_rank {0ull};
    std::size_t input_start_row_this_rank {0ull};
    std::size_t output_start_row_this_rank {0ull};
  };

  NumerologyInfo row_numerology(hep_hpc::ConcatenatedDSInfo const & out_ds_info,
                                std::size_t n_rows_written,
                                std::size_t in_ds_size)
  {
    using std::to_string;
    NumerologyInfo result;
    auto const output_current_size = out_ds_info.current_shape.front();
    auto const incomplete_chunk_size = output_current_size % out_ds_info.chunk_rows;
    auto remaining_rows_this_file = in_ds_size - n_rows_written;
    result.rows_to_write_this_iteration =
      std::min(remaining_rows_this_file - (remaining_rows_this_file % out_ds_info.chunk_rows),
               out_ds_info.buffer_size_rows * n_ranks);
    // Each rank will get either minsize or minsize + 1 chunks to work on.
    auto const rankdiv =
      std::div((long long) (result.rows_to_write_this_iteration / out_ds_info.chunk_rows),
               (long long) n_ranks);
    auto const & minsize = rankdiv.quot;
    auto const & leftovers = rankdiv.rem;
    // Ranks [0, leftovers) get minsize + 1 chunks.
    // Ranks [leftovers, nranks) get minsize chunks.
    auto const chunks_to_write_this_rank = (my_rank < leftovers) ? minsize + 1 : minsize;
    result.rows_to_write_this_rank = chunks_to_write_this_rank * out_ds_info.chunk_rows;
    // Complete an incomplete chunk at the end of output if we need.
    if (incomplete_chunk_size > 0 &&
        result.rows_to_write_this_iteration >= incomplete_chunk_size) {
      // Decrease total rows to write.
      result.rows_to_write_this_iteration -= incomplete_chunk_size;
      if (my_rank == 0) {
        report(3, std::string("Thunking rows_to_write_this_rank from ") +
               to_string(result.rows_to_write_this_rank) + " to " +
               to_string(result.rows_to_write_this_rank - incomplete_chunk_size) +
               ", to account for " + to_string(incomplete_chunk_size) +
               " rows in an incomplete previous chunk.");
        // Decrease rows to write for this rank only.
        result.rows_to_write_this_rank -= incomplete_chunk_size;
      }
    }
    // Tack on some extra rows if that's what we need to complete the file.
    auto const remaining_rows_threshold =
      (incomplete_chunk_size > 0ull) ?
      (2 * out_ds_info.chunk_rows - incomplete_chunk_size) :
      out_ds_info.chunk_rows;
    if (remaining_rows_this_file > 0ull &&
        remaining_rows_this_file <= remaining_rows_threshold) {
      result.rows_to_write_this_iteration += remaining_rows_this_file;
      // Be compact in deciding which rank writes the extra rows.
      auto const rank_to_write_remaining_rows =
        (minsize == 0) ? leftovers : n_ranks - 1;
      if (my_rank == rank_to_write_remaining_rows) {
        report(3, std::string("Thunking rows_to_write_this_rank from ") +
               to_string(result.rows_to_write_this_rank) + " to " +
               to_string(result.rows_to_write_this_rank + remaining_rows_this_file) +
               ", including " + to_string(remaining_rows_this_file) +
               " rows required to complete the current input file.");
        result.rows_to_write_this_rank += remaining_rows_this_file;
      }
    }

    // Calculate start rows for input and output.
    if (my_rank == 0) {
      result.input_start_row_this_rank = n_rows_written;
      result.output_start_row_this_rank = output_current_size;
    } else {
      result.input_start_row_this_rank =
        (n_rows_written - incomplete_chunk_size) +
        ((my_rank < leftovers) ?
         (my_rank * (minsize + 1)) :
         (leftovers + my_rank * minsize)) * out_ds_info.chunk_rows;
      result.output_start_row_this_rank =
        result.input_start_row_this_rank - n_rows_written + output_current_size;
    }
    return result;
  }

}

hep_hpc::HDF5FileConcatenator::
HDF5FileConcatenator(std::string const & output,
                     unsigned int file_mode,
                     std::size_t mem_max,
                     std::string filename_column [[gnu::unused]], // FIXME
                     std::vector<std::string> const & only_groups [[gnu::unused]], // FIXME
                     bool want_filters,
                     bool want_collective_writes,
                     int in_verbosity)
  : mem_max_bytes_(mem_max * 1024 * 1024)
  , want_filters_(want_filters)
  , want_collective_writes_(want_collective_writes)
  , h5out_(open_output_file(output, file_mode))
{
  // Set file-scope parameters.
  verbosity = in_verbosity;
#ifdef HEP_HPC_USE_MPI
  hep_hpc::MPICommunicator world;
  n_ranks = world.size();
  my_rank = world.rank();
#else
  n_ranks = 1;
  my_rank = 0;
#endif
}

int
hep_hpc::HDF5FileConcatenator::
concatFiles(std::vector<std::string> const & inputs [[gnu::unused]])
{
  // FIXME Need to treat filename_column here.

  // Iterate over files:
  for (auto const & input_file_name : inputs) {
    // 1. Open input file
    report(3, std::string("Attempting to open input file ") + input_file_name);
    File input_file(input_file_name, H5F_ACC_RDONLY, {}, maybe_collective_access());

    // 2. Discover and iterate over items.
    report(0, std::string("Processing input file ") + input_file_name);
    // Note: the lambda wrapper is needed to make sure that the C
    // function sees a callback of the correct signature.
    (void) ErrorController::
      call(&H5Ovisit, input_file,
           H5_INDEX_NAME,
           H5_ITER_NATIVE,
           [](hid_t root_id,
              char const * obj_name,
              H5O_info_t const * obj_info,
              void * me) -> herr_t {
             return reinterpret_cast<HDF5FileConcatenator *>(me)->
               visit_item_(root_id, obj_name, obj_info);
           },
           this);

    // FIXME: fill filename_column here.
  }
  report(3, "Completed processing input files");
  return 0;
}

herr_t
hep_hpc::HDF5FileConcatenator::
visit_item_(hid_t root_id,
            char const * obj_name,
            H5O_info_t const * obj_info)
{
  using std::to_string;
  herr_t status = 0;
  // FIXME: handle only_groups.
  switch (obj_info->type) {
  case H5O_TYPE_GROUP:
    report(2, std::string("Ensuring existence of group ") + obj_name + " in output file.");
    {
      Group in_g(ErrorController::call(&H5Oopen_by_addr, root_id, obj_info->addr),
                 ResourceStrategy::handle_tag);
      (void) Group(h5out_,
                   obj_name,
                   Group::OPEN_OR_CREATE_MODE,
                   {},
                   PropertyList(ErrorController::call(&H5Gget_create_plist, in_g),
                                ResourceStrategy::handle_tag)
                  );
    }
    break;
  case H5O_TYPE_DATASET:
  {
    Dataset ds_in(ErrorController::call(&H5Oopen_by_addr, root_id, obj_info->addr),
                  ResourceStrategy::handle_tag);
    status = handle_dataset_(std::move(ds_in), obj_name);
  }
  break;
  case H5O_TYPE_NAMED_DATATYPE:
    report(-1, std::string("Ignoring named datatype ") + obj_name);
    break;
  default:
    report(-2, std::string("Unrecognized HDF5 object type ") +
           to_string(obj_info->type));
    status = -1;
  }
  return status;
}

herr_t
hep_hpc::HDF5FileConcatenator::
handle_dataset_(hdf5::Dataset ds_in, const char * const ds_name)
{
  using std::to_string;
  herr_t status = 0;

  // 1. Discover incoming dataset shape and size.
  report(2, std::string("Examining shape for input dataset ") + ds_name);
  Dataspace in_dataspace(ErrorController::call(&H5Dget_space, ds_in),
                         ResourceStrategy::handle_tag);
  
  if (ErrorController::call(&H5Sis_simple, in_dataspace) <= 0) {
    report(-1, std::string("Ignoring incoming non-simple dataset ") + ds_name);
    return status;
  }

  auto const ndims = ErrorController::call(&H5Sget_simple_extent_ndims, in_dataspace);
  std::vector<hsize_t> in_shape(ndims), in_maxshape(ndims);
  (void) ErrorController::call(&H5Sget_simple_extent_dims,
                               in_dataspace,
                               in_shape.data(),
                               in_maxshape.data());
  auto const in_ds_size = in_shape.front();

  // 2. Check if dataset exists in output. Create and store datasets and
  //    associated information in class state.
  PropertyList in_ds_create_plist(ErrorController::call(&H5Dget_create_plist, ds_in),
                                  ResourceStrategy::handle_tag),
    in_ds_access_plist(ErrorController::call(&H5Dget_access_plist, ds_in),
                       ResourceStrategy::handle_tag);
  bool have_output_ds = false;
  { ScopedErrorHandler disable_hdf5_exceptions;
    report(3, std::string("Checking for existence of dataset ") +
           ds_name + " in output.");
    have_output_ds = (bool) Dataset(h5out_,
                                    ds_name,
                                    PropertyList(in_ds_access_plist));
  }
  auto & out_ds_info = ds_info_[ds_name];
  if (!have_output_ds) { // Does not exist
    report(3, std::string("Creating dataset ") +
           ds_name + " in output.");
    if (!want_filters_) {
      // Deactivate filters in outgoing dataset.
      (void) ErrorController::call(&H5Premove_filter,
                                   in_ds_create_plist,
                                   H5Z_FILTER_ALL);
    }
    auto in_type = ErrorController::call(&H5Dget_type, ds_in);
    out_ds_info.row_size_bytes =
      std::accumulate(in_shape.cbegin() + 1,
                      in_shape.cend(),
                      ErrorController::call(&H5Tget_size, in_type),
                      std::multiplies<std::size_t>());
    (void) ErrorController::call(&H5Pget_chunk, in_ds_create_plist, 1, &out_ds_info.chunk_rows);
    out_ds_info.buffer_size_rows = mem_max_bytes_ / out_ds_info.row_size_bytes;
    // buffer_size_rows should be a multiple of out_ds_info.chunk_rows.
    out_ds_info.buffer_size_rows -= out_ds_info.buffer_size_rows % out_ds_info.chunk_rows;
    out_ds_info.current_shape = in_shape;
    
    out_ds_info.ds = Dataset(h5out_,
                        ds_name,
                        in_type,
                        output_dataspace(in_dataspace),
                        {},
                        std::move(in_ds_create_plist),
                        std::move(in_ds_access_plist));
  }

  if (have_output_ds) { // Resize existing dataset.
    Dataspace out_dataspace(ErrorController::call(&H5Dget_space, out_ds_info.ds),
                            ResourceStrategy::handle_tag);
    auto const out_ndims = ErrorController::call(&H5Sget_simple_extent_ndims, out_dataspace);
    std::vector<hsize_t> out_shape(out_ndims), out_maxshape(out_ndims);
    (void) ErrorController::call(&H5Sget_simple_extent_dims,
                                 out_dataspace,
                                 out_shape.data(),
                                 out_maxshape.data());
    if (!(ndims == out_ndims || std::equal(in_shape.cbegin() + 1,
                                           in_shape.cend(),
                                           out_shape.cbegin() + 1))) {
      report(-2,
             std::string("incoming dataset dimensions are incompatible with outgoing dimensions for dataset ") +
             ds_name);
      status = -1;
      return status;
    }
    auto const new_size_rows = out_ds_info.current_shape.front() + in_ds_size;
    report(1, std::string("resize ") + ds_name + " from " +
           to_string(out_ds_info.current_shape.front()) + " to " +
           to_string(new_size_rows));
    out_ds_info.current_shape.front() = new_size_rows;
    (void) ErrorController::call(&H5Dset_extent,
                                 out_ds_info.ds,
                                 out_ds_info.current_shape.data());
  }

  // 4. Iterate over buffer-sized chunks.
  std::size_t n_rows_written = 0ull;
  while (n_rows_written < in_ds_size) {
    // 4.1 Calculate row numerology.
    auto const numerology [[gnu::unused]] =
      row_numerology(out_ds_info, n_rows_written, in_ds_size);

    // 4.2 Read the correct hyperslab of the input file and copy it to
    //     the corresponding hyperslab of the output.


    // 4.3 Update cursors.
    n_rows_written += numerology.rows_to_write_this_iteration;
    out_ds_info.current_shape.front() +=
      numerology.rows_to_write_this_iteration;
  }
  return status;
}

