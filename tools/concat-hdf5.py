#!/usr/bin/env python
"""Concatenate HDF5 files, combining compatible datasets (those with the
same structure)."""

import h5py
import numpy as np
import sys
import os
from collections import namedtuple
import argparse

def parse_args():
    parser = argparse.ArgumentParser(description='Tabular file concatenator.')
    parser.add_argument('--output', '-o', help='Name of HDF5 output file.')
    parser.add_argument('--append', help='Append to an existing HDF5 file.', action='store_true', default=False)
    parser.add_argument('--mem-max', help='Maximum amount of memory to use to buffer input (MiB).', type=int, default=100)
    parser.add_argument('--filename-column', help='Add a column whose value is extracted from the full input path by the provided regex.', nargs=2)
    parser.add_argument('-v', help='Verbose: repeat for higher verbosity levels', action='count')
    parser.add_argument('inputs', help='Input files to concatenate', nargs='+')
    return parser.parse_args()

class HDF5FileConcatenator:
    def __init__(self, output, append, mem_max, filename_column, verbose):
        self._mem_max_mb = mem_max
        self._filename_column = filename_column
        self._verbose = verbose

        if append:
            mode = 'a'
        else:
            mode = 'w-'

        self._output_file = h5py.File(output, mode)
        self._totalrows = [] # Per dataset.

    def __enter__(self):
        return self;

    def __exit__(self, exc_type, exc_value, traceback):
        self._output_file.close()

    def concatFiles(self, file_list):
        # Iterate over files:
        for input_file_id, input_file_name in enumerate(file_list):
            # 1. Open input file.
            input_file = h5py.File(input_file_name, 'r')
            # 2. Discover and iterate over items.
            input_file.visititems(self._visit_item)

    def _visit_item(self, name, obj):
        # Group or dataset?
        if type(obj) == h5py.Group:
            # Ensure the output file has a corresponding group.
            if self._verbose > 1:
                print("Ensuring existence of group {} in output file.".format(name))
            self._output_file.require_group(name)
        elif type(obj) == h5py.Dataset:
            print("Ignoring existence of found dataset {}".format(name))
            # 3. Discover dataset shape and row size.
            # 4. If first file, create dataset with expansible outer dimension.
            # 5. Calculate how many rows will fit into the current buffer.
            # 6. Iterate over buffer-sized chunks.
            # 7. Obtain the buffer and copy input to correct place in output.
        else:
            print("Found unrecognized item with name {} and type {}".format(name, obj.__class__.name__))


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
