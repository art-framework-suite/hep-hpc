#!/usr/bin/env python
"""Create simple HDF5 file(s) with specified number(s) of rows in a simple dataset for the purpose of checking the numerology of file concatenation."""

from __future__ import print_function
import h5py
import argparse
import numpy as np
import os
import re
import string

def parse_args():
    parser = argparse.ArgumentParser(description='Test file generator for file concatenation.')
    parser.add_argument('nrows', help='Number of rows (multiple values => multiple output_files).', nargs='+')
    parser.add_argument('--chunk-size', '-c', help='Chunk size (rows).', type=int, default=16)
    parser.add_argument('--output-file-stem', '-o', help='Output file stem (use %%i as optional placeholder).', required=True)
    parser.add_argument('--starting-value', '-v' , help='Start value for test data', type=int, default=0)
    return parser.parse_args()

if __name__ == "__main__":

    args = parse_args()

    # For debug.
    np.set_printoptions(threshold = np.inf, linewidth = np.inf)

    format_string = '{{0:0{0}d}}'.format(len(str(len(args.nrows))))

    if args.output_file_stem.find('%i') != -1:
        output_file_stem = args.output_file_stem.replace('%i', format_string)
    else:
        stem, ext = os.path.splitext(args.output_file_stem)
        output_file_stem = stem + '_' + format_string + ext
        
    starting_value = int(args.starting_value)

    for index, nrows in enumerate(args.nrows):
        nrows = int(nrows)
        output_file_name = output_file_stem.format(index)
        output_file = h5py.File(output_file_name, 'w')

        output_file.create_dataset('data',
                                   maxshape = (None,),
                                   chunks = (args.chunk_size,),
                                   compression = "gzip",
                                   compression_opts = 6,
                                   data = np.linspace(starting_value,
                                                      starting_value + nrows,
                                                      num=nrows,
                                                      endpoint=False,
                                                      dtype=np.int32))

        print(output_file_name)

        starting_value += nrows
