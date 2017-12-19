#!/usr/bin/env python
"""Concatenate HDF5 files, combining compatible datasets (those with the
same structure)."""

from __future__ import print_function

import h5py
import numpy as np
import string
import sys
import re
import os
import argparse

try:
    from mpi4py import MPI
    n_ranks = MPI.COMM_WORLD.size
    my_rank = MPI.COMM_WORLD.rank
    WANT_MPI = os.environ["WANT_MPI"] if "WANT_MPI" in os.environ else (n_ranks > 1)
except ImportError:
    MPI = None
    n_ranks = 1
    my_rank = 0
    WANT_MPI = False

def parse_args():
    parser = argparse.ArgumentParser(description='Tabular file concatenator.', prefix_chars='-+')
    parser.add_argument('--output', '-o', help='Name of HDF5 output file.', required=True)
    parser.add_argument('--append', help='Append to an existing HDF5 file.', action='store_true', default=False)
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--with-filters', '-F', help='Use HDF5 filters as appropriate in output',
                       dest='filters', action='store_const', const=True)
    group.add_argument('--without-filters', '+F', help='Do not use HDF5 filters in output',
                       dest='filters', action='store_const', const=False)
    parser.set_defaults(filters=(not WANT_MPI))
    parser.add_argument('--filename-column', help='Add a column whose value is extracted from the full input path by the provided regex replacement (to groups matching regexes only, if provided): args = <column-name> [<regex> [<replacement-expression> [<group-regex>+]]].', nargs='+')
    parser.add_argument('--mem-max', help='Maximum amount of memory to use to buffer input (MiB).', type=int, default=100)
    parser.add_argument('--only-groups', help='Handle only groups matching these regexes, and the datasets within them.',
                        nargs='+')
    parser.add_argument('-v', help='Verbose: repeat for higher verbosity levels', action='count')
    parser.add_argument('inputs', help='Input files to concatenate (use -- to separate from option arguments if necessary)', nargs='+')
    return parser.parse_args()

class HDF5FileConcatenator:
    def __init__(self, output, append, mem_max, filename_column, only_groups, filters, verbose):
        self._mem_max_bytes = mem_max * 1024 * 1024
        self._filename_column = filename_column
        self._want_filters = filters
        self._verbose = verbose
        self._output_file = h5py.File(output, 'a' if append else 'w-', driver = 'mpio', comm=MPI.COMM_WORLD) \
                            if WANT_MPI else h5py.File(output, 'a' if append else 'w-')
        self._ds_out = {}
        # Need tuple keyword to avoid unwanted generator expression!
        self._only_groups = tuple(re.compile(r, re.DEBUG if self._verbose > 3 else 0) for r in only_groups) if only_groups else ()
        self._report(3, "Filters in output are {}".format("enabled" if self._want_filters else "disabled"))

    def __enter__(self):
        return self;

    def __exit__(self, exc_type, exc_value, traceback):
        self._output_file.close()

    def _report(self, level, msg):
        if self._verbose < level: return
        if level == -2:
            LEVEL = "ERROR"
        elif level == -1:
            LEVEL = "WARNING"
        elif level == 0:
            LEVEL = "INFO"
        else:
            LEVEL = "DEBUG({})".format(level)
        print("{}{}: {}".format("({}) ".format(my_rank) if WANT_MPI else "", LEVEL, msg),
              file=sys.stdout if level > -1 else sys.stderr)

    def concatFiles(self, file_list):
        # Define the parameters of the filename column, if we want one.
        if self._filename_column is not None:
            fn_regex = re.compile(self._filename_column[1]) if len(self._filename_column) > 1 else None
            filename_column_values = [ fn_regex.sub(self._filename_column[2] if len(self._filename_column) > 2 else "", f) for f in file_list ] if fn_regex is not None else file_list
            maxchars = max(len(x) for x in filename_column_values)

            filename_column_group_patterns \
                = tuple(re.compile(r) for r in self._filename_column[3:]) \
                if len(self._filename_column) > 3 else ()

        # Iterate over files:
        for input_file_name in file_list:
            self._groups_with_datasets = {}
            filename_column_value = filename_column_values.pop(0) if self._filename_column is not None else None
            if self._filename_column is not None:
                self._report(3, "Calculated value for filename column dataset {} of {} for input file {}.".format(self._filename_column[0], filename_column_value, input_file_name))
            # 1. Open input file.
            input_file = h5py.File(input_file_name, 'r', driver = 'mpio', comm = MPI.COMM_WORLD) \
                         if WANT_MPI else h5py.File(input_file_name, 'r')
            # 2. Discover and iterate over items.
            self._report(0, "Processing input file {}.".format(input_file_name))
            input_file.visititems(self._visit_item)
            # 3. For each group in the output file with datasets with
            #    new rows, populate the appropriate number of rows in
            #    the filename column, if we have one.
            if filename_column_value is not None:
                for ds_group_name in self._groups_with_datasets.keys():
                    if filename_column_group_patterns \
                       and not any(r.match(ds_group_name) for r in filename_column_group_patterns):
                        continue
                    ds_group = self._output_file[ds_group_name]
                    if self._filename_column[0] not in ds_group: # Need output filename column
                        self._report(1, "Creating filename_column dataset {} in group {}.".format(self._filename_column[0], ds_group_name))
                        ds_group.create_dataset(self._filename_column[0],
                                                (0,),
                                                "S{}".format(maxchars),
                                                maxshape = (None,),
                                                chunks = True,
                                                compression = "gzip" if self._want_filters else None,
                                                compression_opts = 6 if self._want_filters else None)
                    filename_column = ds_group[self._filename_column[0]]
                    filename_column_len = filename_column.len()
                    new_rows = max(ds.len() if type(ds) == h5py.Dataset else 0 for ds in ds_group.values()) - \
                               filename_column_len
                    filename_column.resize(filename_column_len + new_rows, 0)
                    filename_column[filename_column_len : filename_column_len + new_rows] = filename_column_value

    def _visit_item(self, name, obj):
        # Group or dataset?
        if type(obj) == h5py.Group:
            if self._only_groups and not any(r.match(obj.name) for r in self._only_groups):
                self._report (3, "Ignoring group {} due to failure to match only_groups regexes.".format(obj.name))
                return
            # Ensure the output file has a corresponding group.
            self._report(2, "Ensuring existence of group {} in output file.".format(obj.name))
            self._output_file.require_group(name)
        elif type(obj) == h5py.Dataset:
            if self._only_groups and not any(r.match(obj.parent.name) for r in self._only_groups):
                self._report(3, "Ignoring dataset {} due to failure to match only_groups regexes for group {}.".format(obj.name, obj.parent.name))
                return
                self._report(1, "Found dataset {} with {} rows.".format(obj.name, obj.len()))
            if self._filename_column is not None and \
               string.split(obj.name, '/')[-1] == self._filename_column[0]:
                raise RuntimeError("Input dataset {} clashes with specified filename column.".format(obj.name))
            self._groups_with_datasets[obj.parent.name] = True
            self._handle_dataset(obj.name, obj)
        else:
            self._report(0, "Ignoring unrecognized item with name {} and type {}".format(name, obj.__class__.name__))

    def _handle_dataset(self, path, ds_in):
        # 3. Discover incoming dataset shape and size.
        shape = ds_in.shape
        input_ds_size = ds_in.len()

        if path not in self._ds_out: # Not seen this dataset before.
            # 4. If first file, create dataset with expansible outer dimension.
            max_shape = list(shape)
            max_shape[0] = None
            new_shape = list(shape)
            new_shape[0] = 0
            self._ds_out[path] = { "ds" : self._output_file.\
                                   create_dataset(path, new_shape, ds_in.dtype,
                                                  chunks = ds_in.chunks,
                                                  compression = ds_in.compression,
                                                  compression_opts = ds_in.compression_opts,
                                                  maxshape = max_shape,
                                                  shuffle = ds_in.shuffle,
                                                  fletcher32 = ds_in.fletcher32,
                                                  scaleoffset = ds_in.scaleoffset,
                                                  fillvalue  = ds_in.fillvalue) if self._want_filters else
                                   self._output_file.\
                                   create_dataset(path, new_shape, ds_in.dtype,
                                                  chunks = ds_in.chunks,
                                                  maxshape = max_shape,
                                                  scaleoffset = ds_in.scaleoffset,
                                                  fillvalue  = ds_in.fillvalue),
                                   "row_size_bytes" : ds_in.size / \
                                   ds_in.len() * ds_in.dtype.itemsize,
                                   "chunk_rows" : ds_in.chunks[0],
                                   "rows_left_this_chunk" : ds_in.chunks[0]
                               }
            self._ds_out[path]["buffer_size_rows"] \
                = self._mem_max_bytes / self._ds_out[path]["row_size_bytes"]
            if ds_in.attrs:
                self._ds_out[path]["ds"].attrs = ds_in.attrs

        # Initialize cursors.
        output_current_size = self._ds_out[path]["ds"].len()
        n_rows_written = 0

        # 5. Do the resize for the whole input dataset.
        self._report(1, "Resize {} from {} to {} rows.".\
                     format(path, output_current_size, \
                            output_current_size + input_ds_size))
        self._ds_out[path]["ds"].resize(output_current_size + input_ds_size, 0)
        self._report(2, "Resize complete.")

        # 6.  Iterate over buffer-sized chunks.
        while n_rows_written < input_ds_size:
            # Calculate row numerology.
            (rows_to_write_this_iteration,
             rows_to_write_this_rank,
             input_start_row_this_rank,
             output_start_row_this_rank) \
                = self._row_numerology(self._ds_out[path],
                                       output_current_size,
                                       n_rows_written,
                                       input_ds_size)
            self._report(4, "Received (rows_to_write_this_iteration, " + \
                         "rows_to_write_this_rank, input_start_row_this_rank, " + \
                         "output_start_row_this_rank = ({}, {}, {}, {}).".\
                         format(rows_to_write_this_iteration,
                                rows_to_write_this_rank,
                                input_start_row_this_rank,
                                output_start_row_this_rank))

            # 7. Write the output slice in the appropriate place 
            self._report(2, "Writing {} rows to dataset {}.".
                         format(rows_to_write_this_rank, path))
            with self._ds_out[path]["ds"].collective:
                self._ds_out[path]["ds"][output_start_row_this_rank : output_start_row_this_rank + rows_to_write_this_rank, ...] \
                    = ds_in[input_start_row_this_rank : input_start_row_this_rank + rows_to_write_this_rank, ...]
            self._report(3, "Write complete.")

            # Update cursors.
            n_rows_written += rows_to_write_this_iteration
            output_current_size += rows_to_write_this_iteration

    def _row_numerology(self, ds_out, output_current_size, n_rows_written, input_ds_size):
        # Calculate numerology for piecewise dataset copy.
        chunk_size_rows = ds_out["chunk_rows"]
        self._report(4, "chunk_size_rows = {}".format(chunk_size_rows))
        buffer_size_rows \
            = ds_out["buffer_size_rows"] - \
            (ds_out["buffer_size_rows"] % chunk_size_rows)
        self._report(4, "buffer_size_rows = {}".format(buffer_size_rows))
        # Need to take account of an incomplete chunk from a previous file.
        incomplete_chunk_size = output_current_size % chunk_size_rows
        self._report(4, "incomplete_chunk_size = {}".format(incomplete_chunk_size))
        remaining_rows_this_file = input_ds_size - n_rows_written
        self._report(4, "remaining_rows_this_file (A) = {}".format(remaining_rows_this_file))
        rows_to_write_this_iteration \
            = min(remaining_rows_this_file - (remaining_rows_this_file % chunk_size_rows),
                  buffer_size_rows * n_ranks - incomplete_chunk_size)
        self._report(4, "rows_to_write_this_iteration = {}".format(rows_to_write_this_iteration))
        # Each rank will get either minsize or minsize+1 chunks to work on.
        minsize, leftovers \
            = divmod(rows_to_write_this_iteration / chunk_size_rows, n_ranks)
        # Ranks [0, leftovers) get minsize+1 chunks.
        # Ranks [leftovers, nranks) get minsize chunks.
        chunks_to_write_this_rank \
            = minsize + 1 if my_rank < leftovers else minsize
        self._report(4, "chunks_to_write_this_rank = {}".format(chunks_to_write_this_rank))
        # Complete an incomplete chunk at the end of output if we need.
        rows_to_write_this_rank = chunks_to_write_this_rank * chunk_size_rows
        self._report(4, "rows_to_write_this_rank = {}".format(rows_to_write_this_rank))
        if incomplete_chunk_size:
            extra_rows = chunk_size_rows - incomplete_chunk_size
            rows_to_write_this_iteration += extra_rows
            if my_rank == 0:
                self._report(3, "Thunking rows_to_write from {} to {}, including {} rows remaining to complete a previous chunk.".\
                             format(rows_to_write_this_rank,
                                    rows_to_write_this_rank + extra_rows,
                                    extra_rows))
                rows_to_write_this_rank += extra_rows
        # Tack on some extra rows if it's what we need to complete the file.
        remaining_rows_this_file \
            = input_ds_size - (n_rows_written + rows_to_write_this_iteration)
        self._report(4, "remaining_rows_this_file (B) = {}".format(remaining_rows_this_file))
        if remaining_rows_this_file > 0 and remaining_rows_this_file < chunk_size_rows:
            rows_to_write_this_iteration += remaining_rows_this_file
            if rows_to_write_this_iteration == 0:
                rank_to_write_remaining_rows = 0
            elif leftovers == 0:
                rank_to_write_remaining_rows = n_ranks - 1
            else:
                rank_to_write_remaining_rows = leftovers
            self._report(4, "rank_to_write_remaining_rows = {}".format(rank_to_write_remaining_rows))
            if my_rank == rank_to_write_remaining_rows:
                self._report(3, "Thunking rows to write from {} to {}, including {} rows required to complete the current input file.".\
                             format(rows_to_write_this_rank,
                                    rows_to_write_this_rank + remaining_rows_this_file,
                                    remaining_rows_this_file))
                rows_to_write_this_rank += remaining_rows_this_file
        self._report(4, "Checkpoint.")
        # Calculate start rows for input and output.
        if my_rank == 0:
            input_start_row_this_rank = n_rows_written
            output_start_row_this_rank = output_current_size
        else:
            input_start_row_this_rank \
                = (n_rows_written - incomplete_chunk_size) \
                + (((my_rank * (minsize + 1)) if my_rank < leftovers else \
                    (leftovers + my_rank * minsize))) * chunk_size_rows
            if incomplete_chunk_size: # Correction.
                input_start_row_this_rank += chunk_size_rows
            output_start_row_this_rank \
                = input_start_row_this_rank - n_rows_written \
                + output_current_size
        self._report(4, "_row_numerology returning (rows_to_write_this_iteration, " + \
                     "rows_to_write_this_rank, input_start_row_this_rank, " + \
                     "output_start_row_this_rank = ({}, {}, {}, {}).".\
                     format(rows_to_write_this_iteration,
                            rows_to_write_this_rank,
                            input_start_row_this_rank,
                            output_start_row_this_rank))
        return (rows_to_write_this_iteration,
                rows_to_write_this_rank,
                input_start_row_this_rank,
                output_start_row_this_rank)


if __name__ == "__main__":
    """Concatenate compatible files into a single output file, combining datasets where appropriate."""

    args = parse_args()

    # For debug.
    np.set_printoptions(threshold = np.inf, linewidth = np.inf)

    with HDF5FileConcatenator(args.output,
                              args.append,
                              args.mem_max,
                              args.filename_column,
                              args.only_groups,
                              args.filters,
                              args.v) as concatenator:
        concatenator.concatFiles(args.inputs)

# Done.

#########################################################################
# Example from https://stackoverflow.com/questions/32961586/combined-hdf5-files-into-single-dataset
#
#        your_data = <get your data from f>
#        total_rows = total_rows + your_data.shape[0]
#        total_columns = your_data.shape[1]

#    if n == 0:
#        #first file; create the dummy dataset with no max shape
#        create_dataset = output_file.create_dataset("Name", (total_rows, total_columns), maxshape=(None, None))
#        #fill the first section of the dataset
#        create_dataset[:,:] = your_data
#        where_to_start_appending = total_rows
#    else:
#        #resize the dataset to accomodate the new data
#        create_dataset.resize(total_rows, axis=0)
#        create_dataset[where_to_start_appending:total_rows, :] = your_data
#        where_to_start_appending = total_rows
