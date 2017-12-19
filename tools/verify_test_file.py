#!/usr/bin/env python
"""Verify a concatenated test file made with concat-hdf5.py from component files created by make_numerology_test_file.py"""

from __future__ import print_function

import h5py
import argparse
import numpy as np
import os
import re
import string
import sys

def parse_args():
    parser = argparse.ArgumentParser(description='Test file generator for file concatenation.')
    parser.add_argument('input_file', help='Name of the input file to verify.')
    parser.add_argument('nrows', help='Expected number of rows (values should increase monotonically from starting value).', type=int)
    parser.add_argument('--starting-value', '-v' , help='Start value for test data', type=int, default=0)
    return parser.parse_args()

if __name__ == "__main__":

    args = parse_args()

    # For debug.
    np.set_printoptions(threshold = np.inf, linewidth = np.inf)

    starting_value = int(args.starting_value)
    nrows = int(args.nrows)
    input = h5py.File(args.input_file.format(), 'r')

    data = input['data']

    if data.size != nrows:
        print("ERROR: Data size mismatch: {} (expected {}).".\
              format(data.size, nrows), file=sys.stderr)
        exit(1)

    ec = 0
    for idx, val in enumerate(np.linspace(starting_value,
                                          starting_value + nrows,
                                          num=nrows,
                                          endpoint=False,
                                          dtype=np.int32)):
        if data[idx] != val:
            print("ERROR: Data mismatch at index {}: {} (expected {}).".\
                  format(idx, data[idx], val), file=sys.stderr)
            ec = 1

    exit(ec)
