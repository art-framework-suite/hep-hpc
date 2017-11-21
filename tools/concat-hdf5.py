#!/usr/bin/env python
"""Concatenate HDF5 files, combining compatible datasets (those with the
same structure)."""

import h5py
import numpy as np
import string
import sys
import re
import os
from collections import namedtuple
import argparse

def parse_args():
    parser = argparse.ArgumentParser(description='Tabular file concatenator.')
    parser.add_argument('--output', '-o', help='Name of HDF5 output file.')
    parser.add_argument('--append', help='Append to an existing HDF5 file.', action='store_true', default=False)
    parser.add_argument('--mem-max', help='Maximum amount of memory to use to buffer input (MiB).', type=int, default=100)
    parser.add_argument('--filename-column', help='Add a column whose value is extracted from the full input path by the provided regex replacement: args = <column-name> [<regex> [<replacement-expression>]].', nargs='+')
    parser.add_argument('-v', help='Verbose: repeat for higher verbosity levels', action='count')
    parser.add_argument('inputs', help='Input files to concatenate (use -- to separate from option arguments if necessary)', nargs='+')
    return parser.parse_args()

class HDF5FileConcatenator:
    def __init__(self, output, append, mem_max, filename_column, verbose):
        self._mem_max_bytes = mem_max * 1024 * 1024
        self._filename_column = filename_column
        self._verbose = verbose

        if append:
            mode = 'a'
        else:
            mode = 'w-'

        self._output_file = h5py.File(output, mode)
        self._ds_out = {}

    def __enter__(self):
        return self;

    def __exit__(self, exc_type, exc_value, traceback):
        self._output_file.close()

    def concatFiles(self, file_list):
        # Define the parameters of the filename column, if we want one.
        if self._filename_column is not None:
            fn_regex = re.compile(self._filename_column[1]) if len(self._filename_column) > 1 else None
            filename_column_values = [ fn_regex.sub(self._filename_column[2] if len(self._filename_column) > 2 else "", f) for f in file_list ] if fn_regex is not None else file_list
            maxchars = max([ len(x) for x in filename_column_values ])

        # Iterate over files:
        for input_file_name in file_list:
            self._groups_with_datasets = {}
            filename_column_value = filename_column_values.pop(0) if self._filename_column is not None else None
            if self._verbose > 1 and self._filename_column is not None:
                print("Calculated value for filename column dataset {} of {} for input file {}.".format(self._filename_column[0], filename_column_value, input_file_name))
            # 1. Open input file.
            input_file = h5py.File(input_file_name, 'r')
            # 2. Discover and iterate over items.
            input_file.visititems(self._visit_item)
            # 3. For each group in the output file with datasets with
            #    new rows, populate the appropriate number of rows in
            #    the filename column, if we have one.
            if filename_column_value is not None:
                for ds_group_name in self._groups_with_datasets.keys():
                    ds_group = self._output_file[ds_group_name]
                    if self._filename_column[0] not in ds_group: # Need output filename column
                        ds_group.create_dataset(self._filename_column[0],
                                                (0),
                                                "S{}".format(maxchars),
                                                maxshape = (None),
                                                chunks = True,
                                                compression = "gzip",
                                                compression_opts = 6)
                    filename_column = ds_group[self._filename_column[0]]
                    filename_column_len = filename_column.len()
                    new_rows = max([ds.len() if type(ds) == h5py.Dataset else 0 for ds in ds_group.values()]) - \
                               filename_column_len
                    filename_column.resize(filename_column_len + new_rows)
                    filename_column[filename_column_len : filename_column_len + new_rows] = filename_column_value

    def _visit_item(self, name, obj):
        # Group or dataset?
        if type(obj) == h5py.Group:
            # Ensure the output file has a corresponding group.
            if self._verbose > 1:
                print("Ensuring existence of group {} in output file.".format(name))
            self._output_file.require_group(name)
        elif type(obj) == h5py.Dataset:
            if self._verbose > 0:
                print("Found dataset {} with {} rows.".format(name, obj.len()))
            if self._filename_column is not None and \
               string.split(obj.name, '/')[-1] == self._filename_column[0]:
                raise RuntimeError("Input dataset {} clashes with specified filename column.".format(obj.name))
            self._groups_with_datasets[obj.parent.name] = True
            self._handle_dataset(name, obj)
        else:
            print("Ignoring unrecognized item with name {} and type {}".format(name, obj.__class__.name__))

    def _handle_dataset(self, path, ds_in):
        # 3. Discover dataset shape and row size.
        shape = ds_in.shape
        len_in = ds_in.len()
        n_rows_written = 0
        if path not in self._ds_out: # Not seen this dataset before.
            # 4. If first file, create dataset with expansible outer dimension.
            max_shape = list(shape)
            max_shape[0] = None
            new_shape = list(shape)
            new_shape[0] = 0
            self._ds_out[path] = { "ds" : self._output_file.create_dataset(path, new_shape, ds_in.dtype,
                                                                           chunks = ds_in.chunks,
                                                                           compression = ds_in.compression,
                                                                           compression_opts = ds_in.compression_opts,
                                                                           maxshape = max_shape,
                                                                           shuffle = ds_in.shuffle,
                                                                           fletcher32 = ds_in.fletcher32,
                                                                           scaleoffset = ds_in.scaleoffset,
                                                                           fillvalue  = ds_in.fillvalue),
                                   "row_size_bytes" : ds_in.size / ds_in.len() * ds_in.dtype.itemsize,
                               }
            self._ds_out[path]["buffer_size_rows"] = self._mem_max_bytes / self._ds_out[path]["row_size_bytes"]
            if ds_in.attrs:
                self._ds_out[path]["ds"].attrs = ds_in.attrs

        # 5. Iterate over buffer-sized chunks.
        while n_rows_written < len_in:
            output_current_size = self._ds_out[path]["ds"].len()
            rows_to_write = min(ds_in.len(), self._ds_out[path]["buffer_size_rows"])
            # 6. Resize the output dataset appropriately.
            if self._verbose > 1:
                print("Resize {} from {} to {} rows.".format(path, output_current_size, output_current_size + rows_to_write))
            self._ds_out[path]["ds"].resize(output_current_size + rows_to_write, 0)
            # 7. Write the output slice in the appropriate place 
            if self._verbose > 1:
                print("Writing {} rows to dataset {}.".format(rows_to_write, path))
            self._ds_out[path]["ds"][output_current_size : output_current_size + rows_to_write, ...] = ds_in[n_rows_written : n_rows_written + rows_to_write, ...]
            # Update cursor.
            n_rows_written += rows_to_write

if __name__ == "__main__":
    """Concatenate compatible files into a single output file, combining datasets where appropriate."""

    args = parse_args()

    if args.output is None:
        raise RuntimeError("Output file specification is obligatory.")

    # For debug.
    np.set_printoptions(threshold = np.inf, linewidth = np.inf)

    with HDF5FileConcatenator(args.output,
                              args.append,
                              args.mem_max,
                              args.filename_column,
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
