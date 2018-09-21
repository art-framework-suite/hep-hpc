#!/usr/bin/env python
"""Demonstrate issue with collective writes when at least one rank has nothing to do."""

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
    parser = argparse.ArgumentParser(description='Collective write hang demonstrator.', prefix_chars='-+')
    parser.add_argument('output', help='Name of HDF5 output file.')
    parser.add_argument('nrows', help='number_of_rows', type=int, default=n_ranks-1, nargs='?')
    return parser.parse_args()

class dummy_context_mgr:
    def __enter__(self):
        return None
    def __exit__(self, exc_type, exc_value, traceback):
        return False

if __name__ == "__main__":

    args = parse_args()

    nrows = int(args.nrows)

    output_file = h5py.File(args.output, 'w', driver='mpio', comm=MPI.COMM_WORLD) if WANT_MPI \
                  else h5py.File(args.output, 'w')

    dataset = output_file.create_dataset('data', shape=(nrows,), maxshape=(None,), chunks = True)

    data = np.linspace(0, nrows, num=nrows, endpoint=False, dtype=np.int32)

    iterations = 0
    while iterations * n_ranks < nrows:
        start = iterations * n_ranks + my_rank
        end = start + 1 if start < nrows else start

        with dataset.collective if WANT_MPI else dummy_context_mgr():
            dataset[start:end] = data[start:end]

        iterations += 1
