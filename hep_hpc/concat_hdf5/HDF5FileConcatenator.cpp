#include "hep_hpc/concat_hdf5/HDF5FileConcatenator.hpp"

#include "hep_hpc/detail/config.hpp"
#ifdef HEP_HPC_USE_MPI
#include "hep_hpc/MPI/MPICommunicator.hpp"
#endif

#include "hep_hpc/concat_hdf5/maybe_report_rank.hpp"
#include "hep_hpc/hdf5/Dataset.hpp"
#include "hep_hpc/hdf5/Datatype.hpp"
#include "hep_hpc/hdf5/Group.hpp"
#include "hep_hpc/hdf5/PropertyList.hpp"
#include "hep_hpc/hdf5/ResourceStrategy.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

extern "C" {
#include "sys/stat.h"
#include "H5FDmpio.h"
#include "H5Spublic.h"
}

#include <algorithm>
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

  // Ascertain with minimum fuss whether a file exists.
  bool file_exists(std::string file_name)
  {
    struct stat buffer;
    return (stat(file_name.c_str(), &buffer) == 0);
  }

  // Error and debug reporting.
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

  // Set independent or collective access on the file.
  PropertyList
  maybe_collective_access()
  {
    PropertyList file_access_properties(H5P_FILE_ACCESS);
#ifdef HEP_HPC_USE_MPI
    if (n_ranks > 1) {
      report(1, "Setting access properties to use MPI I/O.");
      (void) ErrorController::call(&H5Pset_fapl_mpio,
                                   file_access_properties,
                                   MPI_COMM_WORLD,
                                   MPI_INFO_NULL);
    }
#endif
    return file_access_properties;
  }

  // Get the number of rows from a dataspace.
  hsize_t
  num_rows_from_dspace(Dataspace const & in_dspace)
  {
    hsize_t result = 0ull;

    auto const ndims =
      ErrorController::call(&H5Sget_simple_extent_ndims, in_dspace);

    std::vector<hsize_t> in_shape(ndims), in_maxshape(ndims);
    (void) ErrorController::call(&H5Sget_simple_extent_dims,
                                 in_dspace,
                                 in_shape.data(),
                                 in_maxshape.data());

    result = in_shape.front();
    return result;
 }

  // Open the output file.
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

  // Select the correct hyperslab for IO in a dataspace,
  // returning the corresponding memory dataspace.
  Dataspace
  prepare_dspace(Dataspace & file_dspace,
                    hsize_t const start_row,
                    hsize_t const rows_for_io)
  {
    Dataspace mem_dspace; // Result.
    auto const ndims =
      ErrorController::call(&H5Sget_simple_extent_ndims,
                            file_dspace);
    std::vector<hsize_t> shape(ndims), maxshape(ndims);
    (void) ErrorController::call(&H5Sget_simple_extent_dims,
                                 file_dspace,
                                 shape.data(),
                                 maxshape.data());
    std::vector<hsize_t> offsets(ndims),
      block_count(ndims),
      n_elements(shape);
    offsets.front() = start_row;
    std::fill(block_count.begin(), block_count.end(), 1);
    n_elements.front() = rows_for_io;
    if (rows_for_io > 0) {
      (void) ErrorController::call(&H5Sselect_hyperslab,
                                   file_dspace,
                                   H5S_SELECT_SET,
                                   offsets.data(),
                                   nullptr,
                                   block_count.data(),
                                   n_elements.data());
      mem_dspace =
        Dataspace(ndims, n_elements.data(), n_elements.data());
    } else {
      (void) ErrorController::call(&H5Sselect_none, file_dspace);
      mem_dspace = Dataspace(H5S_NULL);
    }
    return mem_dspace;
  }

  // Structure to hold the calculated numerology for an I/O operation.
  struct NumerologyInfo {
    hsize_t rows_to_write_this_iteration {0ull};
    hsize_t rows_to_write_this_rank {0ull};
    hsize_t input_start_row_this_rank {0ull};
    hsize_t output_start_row_this_rank {0ull};
  };

  // Calculate the numerology for an I/O operation.
  NumerologyInfo
  row_numerology(hep_hpc::ConcatenatedDSInfo const & out_ds_info,
                 hsize_t const n_rows_written_this_input,
                 hsize_t const in_ds_size)
  {
    using std::to_string;
    NumerologyInfo result;
    // Do we have an incomplete chunk in the output?
    auto const incomplete_chunk_size =
      out_ds_info.n_rows_written_total % out_ds_info.chunk_rows;
    // How many rows left in this input file?
    auto remaining_rows_this_file =
      in_ds_size - n_rows_written_this_input;
    // How many whole chunks can we write per rank?
    auto const max_chunks_per_rank =
      out_ds_info.buffer_size_rows / out_ds_info.chunk_rows;
    // How many rows to write this iteration?
    result.rows_to_write_this_iteration =
      std::min(remaining_rows_this_file,
               max_chunks_per_rank * out_ds_info.chunk_rows * n_ranks);
    // Trim to integral chunks and worry about the remainder later.
    result.rows_to_write_this_iteration -=
      (result.rows_to_write_this_iteration % out_ds_info.chunk_rows);
    // Each rank will get either minsize or minsize + 1 chunks to work on.
    auto const rankdiv =
      std::div((long long) (result.rows_to_write_this_iteration /
                            out_ds_info.chunk_rows),
               (long long) n_ranks);
    auto const & minsize = rankdiv.quot;
    auto const & leftovers = rankdiv.rem;
    // Ranks [0, leftovers) get minsize + 1 chunks.
    // Ranks [leftovers, nranks) get minsize chunks.
    auto const chunks_to_write_this_rank =
      (my_rank < leftovers) ? minsize + 1 : minsize;
    result.rows_to_write_this_rank =
      chunks_to_write_this_rank * out_ds_info.chunk_rows;
    // Complete an incomplete chunk at the end of output if we need.
    if (incomplete_chunk_size > 0 &&
        result.rows_to_write_this_iteration >= incomplete_chunk_size) {
      // Decrease total rows to write.
      result.rows_to_write_this_iteration -= incomplete_chunk_size;
      if (my_rank == 0) {
        report(3,
               std::string("Thunking rows_to_write_this_rank from ") +
               to_string(result.rows_to_write_this_rank) + " to " +
               to_string(result.rows_to_write_this_rank -
                         incomplete_chunk_size) + ", to account for " +
               to_string(incomplete_chunk_size) +
               " rows in an incomplete previous chunk.");
        // Decrease rows to write for this rank only.
        result.rows_to_write_this_rank -= incomplete_chunk_size;
      }
    }
    // Tack on some extra rows if that's what we need to complete the file.
    remaining_rows_this_file -= result.rows_to_write_this_iteration;
    hsize_t extra_rows_to_write = 0ull;
    int rank_to_write_remaining_rows = 0;
    if (remaining_rows_this_file > 0ull &&
        remaining_rows_this_file < out_ds_info.chunk_rows) {
      extra_rows_to_write = remaining_rows_this_file;
      // Be compact in deciding which rank writes the extra rows.
      rank_to_write_remaining_rows =
        (minsize == 0) ? leftovers : n_ranks - 1;
    } else if (result.rows_to_write_this_iteration == 0) {
      extra_rows_to_write =
        std::min(remaining_rows_this_file,
                 out_ds_info.buffer_size_rows);
    }
    if (extra_rows_to_write > 0ull) {
      result.rows_to_write_this_iteration += extra_rows_to_write;
      // If there's room in the buffer for the remaining rows, write
      // them, otherwise we get to do another iteration.
      if (my_rank == rank_to_write_remaining_rows &&
          ! (result.rows_to_write_this_rank + extra_rows_to_write >
             out_ds_info.buffer_size_rows)) {
        report(3, std::string("Thunking rows_to_write_this_rank from ") +
               to_string(result.rows_to_write_this_rank) + " to " +
               to_string(result.rows_to_write_this_rank + extra_rows_to_write) +
               ".");
        result.rows_to_write_this_rank += extra_rows_to_write;
      }
    }

    // Calculate start rows for input and output.
    if (my_rank == 0) {
      result.input_start_row_this_rank = n_rows_written_this_input;
      result.output_start_row_this_rank =
        out_ds_info.n_rows_written_total;
    } else {
      // Must take account of leftovers.
      hsize_t const offset =
        ((my_rank < leftovers) ?
         (my_rank * (minsize + 1)) :
         (leftovers + my_rank * minsize)) * out_ds_info.chunk_rows -
        incomplete_chunk_size;

      result.input_start_row_this_rank =
        n_rows_written_this_input + offset;
      result.output_start_row_this_rank =
        out_ds_info.n_rows_written_total + offset;
    }
    return result;
  }

  // Return the parent group of a dataset or group
  std::string
  parent_group(std::string const & child)
  {
    auto const slash_pos = child.rfind('/');
    std::string
      parent = (slash_pos != std::string::npos) ?
      child.substr(0, slash_pos) : ".";
    return parent;
  }

  // Match a group name against a bunch of regexes, returning true if
  // any match.
  bool
  match_group_against_regexes(std::string const & group_name,
                              std::vector<std::regex> const & only_groups)
  {
    return only_groups.size() == 0 ||
      std::any_of(only_groups.cbegin(),
                  only_groups.cend(),
                  [group_name](std::regex const & re)
                  { return std::regex_match(group_name, re); });
  }

  // Quick calculation if we still have rows available in the output.
  hsize_t
  rows_available(hep_hpc::ConcatenatedDSInfo const & info,
                 long long const max_rows)
  {
    hsize_t result = ((hsize_t) max_rows) - info.n_rows_written_total;
    return result;
  }

  // Prepare output dataspace based on input dataspace and other constraints.
  Dataspace
  output_dspace(Dataspace const & in_dspace,
                hep_hpc::ConcatenatedDSInfo const & info,
                long long const max_rows)
  {
    Dataspace result = in_dspace;
    auto const ndims = ErrorController::call(&H5Sget_simple_extent_ndims, result);
    std::vector<hsize_t> shape(ndims), maxshape(ndims);
    (void) ErrorController::call(&H5Sget_simple_extent_dims,
                                 result,
                                 shape.data(),
                                 maxshape.data());
    // The output size is unconstrained.
    maxshape.front() = H5S_UNLIMITED;
    // Modify initial shape based on user-set constraint, if present.
    shape.front() = std::min(rows_available(info, max_rows), shape.front());

    // Diagnostics.
    std::ostringstream out;
    out << "Shape: (";
    std::copy(shape.cbegin(), shape.cend(), std::ostream_iterator<hsize_t>(out, ", "));
    out.seekp(-2, std::ios_base::cur);
    out << ").";
    report(4, out.str());
    out.clear();
    out << "Max shape: (";
    std::copy(maxshape.cbegin(), maxshape.cend(), std::ostream_iterator<hsize_t>(out, ", "));
    out.seekp(-2, std::ios_base::cur);
    out << ").";
    report(4, out.str());

    // Set the extent in the dataspace.
    (void) ErrorController::call(&H5Sset_extent_simple,
                                 result,
                                 ndims,
                                 shape.data(),
                                 maxshape.data());
    report(4, "Completed setting output dataspace extent.");
    return result;
  }

  Dataset
  create_or_open_and_resize_dataset(std::string const ds_name,
                                    hep_hpc::ConcatenatedDSInfo & out_ds_info,
                                    Dataset const & in_ds,
                                    File & h5out,
                                    bool const want_filters,
                                    std::size_t mem_max_bytes,
                                    long long const max_rows)
  {
    // For number to string conversions.
    using std::to_string;

    // Result.
    Dataset out_dset;

    // Interrogate input dataset.
    PropertyList in_ds_access_plist(ErrorController::call(&H5Dget_access_plist, in_ds),
                                    ResourceStrategy::handle_tag);
    Dataspace const in_dspace(ErrorController::call(&H5Dget_space, in_ds),
                              ResourceStrategy::handle_tag);
    auto const ndims =
      ErrorController::call(&H5Sget_simple_extent_ndims, in_dspace);

    std::vector<hsize_t> in_shape(ndims), in_maxshape(ndims);
    (void) ErrorController::call(&H5Sget_simple_extent_dims,
                                 in_dspace,
                                 in_shape.data(),
                                 in_maxshape.data());

    auto const in_ds_size = in_shape.front();

    Datatype const in_type (ErrorController::call(&H5Dget_type, in_ds));

    // Create or open and resize.
    if (out_ds_info.row_size_bytes == 0ull) {
      // Output dataset does not exist.

      // Grab the create property list.
      PropertyList in_ds_create_plist(ErrorController::call(&H5Dget_create_plist, in_ds),
                                      ResourceStrategy::handle_tag);

      if (!want_filters) {
        // Deactivate filters in outgoing dataset.
        (void) ErrorController::call(&H5Premove_filter,
                                     in_ds_create_plist,
                                     H5Z_FILTER_ALL);
      }
      // Save parent group's name.
      out_ds_info.parent = parent_group(ds_name);

      // Calculate the size of a row in bytes.
      out_ds_info.row_size_bytes =
        std::accumulate(in_shape.cbegin() + 1,
                        in_shape.cend(),
                        ErrorController::call(&H5Tget_size, in_type),
                        std::multiplies<hsize_t>());
      report(3, std::string("Calculated row_size_bytes = ") +
             to_string(out_ds_info.row_size_bytes) + " in dataset " + ds_name);

      // Store the chunk size in rows.
      (void) ErrorController::call(&H5Pget_chunk,
                                   in_ds_create_plist,
                                   1,
                                   &out_ds_info.chunk_rows);

      // Calculate the buffer size in rows.
      out_ds_info.buffer_size_rows = mem_max_bytes / out_ds_info.row_size_bytes;
      report(3, std::string("Calculated buffer_size_rows = ") +
             to_string(out_ds_info.buffer_size_rows) + " in dataset " + ds_name);

      if (out_ds_info.buffer_size_rows == 0) {
        throw
          std::runtime_error(std::string("Unable to write a complete row of dataset ") +
                             ds_name +
                             " (" +
                             to_string(out_ds_info.row_size_bytes) +
                             " B) due to configured buffer size of " +
                             to_string(mem_max_bytes) + " B.");
      } else if (out_ds_info.buffer_size_rows < out_ds_info.chunk_rows) {
        report (-1, std::string("Configured buffer size allows for only ") +
                to_string(out_ds_info.buffer_size_rows) +
                " rows from dataset " +
                ds_name +
                ",\nwhich is less than one complete chunk (" +
                to_string(out_ds_info.chunk_rows) +
                " rows).\n I/O will be restricted to rank 0 only!");
      }

      // Create the dataset.
      report(2, std::string("Creating dataset ") +
             ds_name + " in output.");

      Dataspace out_dspace(output_dspace(in_dspace, out_ds_info, max_rows));
      report(4, "out_dspace ready.");
      out_dset = Dataset(h5out,
                         ds_name,
                         in_type,
                         std::move(out_dspace),
                         {},
                         std::move(in_ds_create_plist),
                         std::move(in_ds_access_plist));
      report(4, std::string("Created dataset ") +
             ds_name + " in output.");
    } else {
      // Open the output dataset.
      out_dset = Dataset(h5out, ds_name, std::move(in_ds_access_plist));

      // Resize output dataset.
      hsize_t const rows_threshold =
        std::min(rows_available(out_ds_info, max_rows), in_ds_size);

      Dataspace out_dspace(ErrorController::call(&H5Dget_space, out_dset),
                           ResourceStrategy::handle_tag);

      auto const out_ndims = ErrorController::call(&H5Sget_simple_extent_ndims, out_dspace);
      std::vector<hsize_t> out_shape(out_ndims), out_maxshape(out_ndims);
      (void) ErrorController::call(&H5Sget_simple_extent_dims,
                                   out_dspace,
                                   out_shape.data(),
                                   out_maxshape.data());
      if (!(ndims == out_ndims || std::equal(in_shape.cbegin() + 1,
                                             in_shape.cend(),
                                             out_shape.cbegin() + 1))) {
        throw std::runtime_error(
          std::string("incoming dataset dimensions are incompatible with outgoing dimensions for dataset ") +
          ds_name);
      }
      auto const new_size_rows = out_shape.front() + rows_threshold;
      report(1, std::string("resize ") + ds_name + " from " +
             to_string(out_shape.front()) + " to " +
             to_string(new_size_rows));
      out_shape.front() = new_size_rows;
      (void) ErrorController::call(&H5Dset_extent,
                                   out_dset,
                                   out_shape.data());
    }
    return out_dset;
  }

}

hep_hpc::HDF5FileConcatenator::
HDF5FileConcatenator(std::string const & output,
                     unsigned int file_mode,
                     long long max_rows,
                     std::size_t mem_max_bytes,
                     FilenameColumnInfo filename_column_info,
                     std::vector<std::regex> const & only_groups,
                     bool want_filters,
                     bool want_collective_writes,
                     int in_verbosity)
  : max_rows_(max_rows)
  , mem_max_bytes_(mem_max_bytes)
  , want_filters_(want_filters)
  , want_collective_writes_(want_collective_writes)
  , filename_column_info_(std::move(filename_column_info))
  , only_groups_(only_groups)
  , buffer_(mem_max_bytes_)
  , h5out_()
  , ds_info_()
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
  // Must wait until n_ranks & my_rank are initialized.
  h5out_ = open_output_file(output, file_mode);
}

int
hep_hpc::HDF5FileConcatenator::
concatFiles(std::vector<std::string> const & inputs)
{
  // Initialize filename column data from input filenames (as given).
  if (!filename_column_info_.column_name().empty()) {
    if (filename_column_info_.has_regex()) {
      for (auto const & input : inputs) {
        filename_column_data_.
          emplace_back(std::regex_replace(input,
                                          filename_column_info_.regex(),
                                          filename_column_info_.replacement()));
      }
    } else {
      filename_column_data_ = inputs;
    }
  }

  // Iterate over files:
  auto fn_column_val_iterator = filename_column_data_.cbegin();
  for (auto const & input_file_name : inputs) {
    // 1. Open input file
    report(2, std::string("Attempting to open input file ") + input_file_name);
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

    // Fill filename_column.
    hsize_t max_fn_column_val_size =
      (filename_column_data_.size() > 0) ?
      std::max_element(filename_column_data_.cbegin(),
                       filename_column_data_.cend(),
                       [](std::string const & l,
                          std::string const & r)
                       { return l.size() < r.size(); })->size() : 0;
    Datatype s_type(H5Tcopy(H5T_C_S1));
    H5Tset_size(s_type, max_fn_column_val_size);
    for (auto & group_info : group_filename_column_ds_size_) {
      auto const & group_path = group_info.first;
      auto & ds_info = group_info.second;
      if (ds_info.required_size == 0) {
        // Don't need this dataset (yet).
        continue;
      }
      if (!ds_info.ds) {
        report(2, std::string("Creating filename column dataset ") +
               group_path + '/' +
               filename_column_info_.column_name() + " in output.");
        // Create dataset of required size.
        hsize_t const ds_size = ds_info.required_size;
        hsize_t const ds_maxsize = H5S_UNLIMITED;
        hsize_t const chunk_size = 128;
        PropertyList cprops(H5P_DATASET_CREATE);
        cprops(&H5Pset_chunk, 1, &chunk_size);
        if (want_filters_) {
          cprops(&H5Pset_deflate, 6);
        }
        ds_info.name =
          group_path + '/' + filename_column_info_.column_name();
        ds_info.ds =
          Dataset(h5out_,
                  ds_info.name,
                  s_type,
                  Dataspace(1, &ds_size, &ds_maxsize),
                  {},
                  cprops);
      } else {
        // Resize existing dataspace.
        (void) ErrorController::call(&H5Dset_extent,
                                     ds_info.ds,
                                     &ds_info.required_size);    
      }
      // Fill the required section of the dataspace with the expected
      // value for this input file.
      static hsize_t const block_count  = 1;
      Datatype new_s_type(H5Tcopy(H5T_C_S1));
      H5Tset_size(new_s_type, fn_column_val_iterator->size());
      auto const n_new_elements = ds_info.required_size - ds_info.current_size;
      std::unique_ptr<uint8_t[]> buf(new uint8_t[max_fn_column_val_size * n_new_elements]);
      Dataspace mem_dspace(1, &n_new_elements, &n_new_elements);
      // Fill a memory buffer with the data to write.
      report(4, std::string("Fill buffer for filename column dataset ") + ds_info.name + ".");
      (void) ErrorController::call(&H5Dfill,
                                   fn_column_val_iterator->c_str(),
                                   new_s_type,
                                   buf.get(),
                                   s_type,
                                   mem_dspace);
      Dataspace file_dspace(1, &ds_info.required_size, &ds_info.required_size);
      // Write our memory buffer to the dataset.
      report(4, std::string("Write buffer for filename column dataset ") + ds_info.name + ".");
      (void) ErrorController::call(&H5Sselect_hyperslab,
                                   file_dspace,
                                   H5S_SELECT_SET,
                                   &ds_info.current_size,
                                   nullptr,
                                   &block_count,
                                   &n_new_elements);
      ds_info.ds.write(s_type,
                       buf.get(),
                       std::move(mem_dspace),
                       std::move(file_dspace),
                       transfer_properties_());
      // Update current size.
      ds_info.current_size = ds_info.required_size;
    }
    // Bump to next value for filename column dataset.
    ++fn_column_val_iterator;
  }
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
  switch (obj_info->type) {
  case H5O_TYPE_GROUP:
    if (match_group_against_regexes(obj_name, only_groups_)) {
      report(2, std::string("Ensuring existence of group ") + obj_name + " in output file.");
      // Make sure the group exists in the output.
      Group in_g(ErrorController::call(&H5Oopen_by_addr, root_id, obj_info->addr),
                 ResourceStrategy::handle_tag);

      (void)
        Group(h5out_,
              obj_name,
              Group::OPEN_OR_CREATE_MODE,
              {},
              PropertyList(ErrorController::call(&H5Gget_create_plist, in_g),
                           ResourceStrategy::handle_tag)
             );

      // If we decide it needs a filename column, keep track of how many
      // rows it should have.
      if (!filename_column_info_.column_name().empty() &&
          match_group_against_regexes(obj_name,
                                       filename_column_info_.group_regexes())) {
        group_filename_column_ds_size_[obj_name].required_size = 0ull;
      }
    } else if (strcmp(".", obj_name) != 0) {
      report(3, std::string("Ignoring group ") + obj_name +
             " due to failure to match --only-groups specification.");
    }
    break;
  case H5O_TYPE_DATASET:
    if (match_group_against_regexes(parent_group(obj_name),
                                    only_groups_)) {
      Dataset
        in_ds(ErrorController::call(&H5Oopen_by_addr,
                                    root_id,
                                    obj_info->addr),
              ResourceStrategy::handle_tag);
      status = handle_dataset_(std::move(in_ds), obj_name);
    } else {
      report(3, std::string("Ignoring dataset ") + obj_name +
             " due to failure of containing group to match --only-groups specification.");
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
handle_dataset_(hdf5::Dataset in_ds, std::string const ds_name)
{
  // For number to string conversions.
  using std::to_string;

  // Result.
  herr_t status = 0;

  // 1. Discover incoming dataset shape and size.
  report(2, std::string("Examining shape for input dataset ") + ds_name);
  Dataspace in_dspace(ErrorController::call(&H5Dget_space, in_ds),
                         ResourceStrategy::handle_tag);

  if (ErrorController::call(&H5Sis_simple, in_dspace) <= 0) {
    report(-1, std::string("Ignoring incoming non-simple dataset ") + ds_name);
    return status;
  }

  Datatype const in_type (ErrorController::call(&H5Dget_type, in_ds));

  auto const in_ds_size = num_rows_from_dspace(in_dspace);

  auto & out_ds_info = ds_info_[ds_name];

  // 2. Check if dataset exists in output. Create and store datasets and
  //    associated information in class state.
  Dataset out_dset =
    create_or_open_and_resize_dataset(ds_name,
                                      out_ds_info,
                                      in_ds,
                                      h5out_,
                                      want_filters_,
                                      mem_max_bytes_,
                                      max_rows_);

  // How many rows can be written into the dataset?
  hsize_t const rows_threshold =
    std::min(rows_available(out_ds_info, max_rows_), in_ds_size);

  // Get an up-to-date copy of the output dataspace,
  Dataspace out_dspace =
    Dataspace(ErrorController::call(&H5Dget_space, out_dset),
              ResourceStrategy::handle_tag);

  // 4. Iterate over buffer-sized chunks.
  auto n_rows_written_this_input = 0ull;

  while (n_rows_written_this_input < rows_threshold) {
    // 4.1 Calculate row numerology.
    auto const numerology =
      row_numerology(out_ds_info,
                     n_rows_written_this_input,
                     rows_threshold);

    // 4.2 Read the correct hyperslab of the input file and copy it to
    //     the corresponding hyperslab of the output.

    // 4.2.1 Prepare the input dataspace.
    auto mem_dspace =
      prepare_dspace(in_dspace,
                        numerology.input_start_row_this_rank,
                        numerology.rows_to_write_this_rank);

    // 4.2.2 Execute the read.
    report(4, std::string("Reading ") +
           to_string(numerology.rows_to_write_this_rank) + 
           " rows from [" +
           to_string(numerology.input_start_row_this_rank) +
           ", " +
           to_string(numerology.input_start_row_this_rank +
                     numerology.rows_to_write_this_rank) +
           ") in dataset " + ds_name + ".");
    (void) in_ds.read(in_type,
                      buffer_.data(),
                      std::move(mem_dspace),
                      in_dspace,
                      transfer_properties_());
    // 4.2.3 Prepare the dataspaces.
    mem_dspace =
      prepare_dspace(out_dspace,
                        numerology.output_start_row_this_rank,
                        numerology.rows_to_write_this_rank);

    // 4.2.4 Execute the write.
    report(4, std::string("Writing ") +
           to_string(numerology.rows_to_write_this_rank) + 
           " rows to [" +
           to_string(numerology.output_start_row_this_rank) +
           ", " +
           to_string(numerology.output_start_row_this_rank +
                     numerology.rows_to_write_this_rank) +
           ") in dataset " + ds_name + ".");
    (void) out_dset.write(in_type,
                          buffer_.data(),
                          std::move(mem_dspace),
                          out_dspace,
                          transfer_properties_());

    // 4.3 Update cursors.
    n_rows_written_this_input += numerology.rows_to_write_this_iteration;
    out_ds_info.n_rows_written_total +=
      numerology.rows_to_write_this_iteration;
  }

  // Update the required size of the filename column dataset, if
  // necessary.
  auto const group_size_iter =
    group_filename_column_ds_size_.find(out_ds_info.parent);
  if (group_size_iter != group_filename_column_ds_size_.cend()) {
    group_size_iter->second.required_size =
      std::max(group_size_iter->second.required_size,
               out_ds_info.n_rows_written_total);
  }

  (void) ErrorController::call(&H5Fflush, h5out_, H5F_SCOPE_LOCAL);

  // Done.
  return status;
}

PropertyList
hep_hpc::HDF5FileConcatenator::
transfer_properties_()
{
#ifdef HEP_HPC_USE_MPI
  PropertyList xfer_properties(H5P_DATASET_XFER);
  if (want_collective_writes_) {
    (void) ErrorController::call(&H5Pset_dxpl_mpio,
                                 xfer_properties,
                                 H5FD_MPIO_COLLECTIVE);
  }
#else
  PropertyList xfer_properties;
#endif
  return xfer_properties;
}
