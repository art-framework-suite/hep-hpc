# README #

This code was produced as part of a Fermilab LDRD project studying the
use of HDF5 for HEP event data.

See the
[original project wiki](https://bitbucket.org/mpaterno/hdffilestructurestudy/wiki).

## What is this repository for? ##

This repository includes utilities for resource and error management of
HDF5 entities, and a facility (Ntuple) for persistence of simple tabular
data in HDF5 files with row-wise fill semantics and column-wise read.

## Requirements ##

* Linux (tested on RHEL6/7-related, should work on Ubuntu 16.04) or Mac
  (tested on Sierra).
* CMake.
* A modern C++ compiler (C++17 supported, C++14-compliant preferred,
  C++11 compliance required). If you require C++11 compatibility rather
  than C++14 or better, Boost is required to provide facilities that
  would otherwise be provided by the standard library.
* A modern HDF5 distribution (1.10.0+).
* Optionally, MPI distribution (tested with MPICH, likely to be
  problematic with OpenMPI).
* For reading the data so produced, there are Python and R packages
  which should be very straightforward to use (e.g. h5py, pandas,
  h5). See notes below with regard to package compatibility, however.

N.B. due to ABI compatibility restrictions with respect to both C++ and
Fortran >= 90, all dependencies using either of these languages
(e.g. HDF5 and MPI with most common configurations) must have been
compiled with the same compiler. Pure C and/or Fortan 77 dependencies do
not suffer this restriction. Packages used for data reading will have no
link dependency on the compiled code herein, so you are not constrained
to use the same compiler (for instance). However your data reading
packages will have to be consistent within themselves in their use of
(e.g.) compiler, HDF5, MPI, etc.

## How do I get set up? ##

### Mac ###

The system has been tested with a native installation on Sierra. The
following describes getting everything up through h5py, including the
optional MPI for parallel I/O with HDF5.

1. Python from Homebrew.
2. MPI from Homebrew, specifically `mpich`. OpenMPI causes failures in
   some uses of parallel I/O in `h5py`.
3. HDF5 1.10+, see below.
4. `mpi4py`, `numpy`, and `six`, installed using `pip`. 
5. `h5py` installed from the installation tarball, after running `python
   setup.py configure --mpi` to force creation of the MPI-aware version
   of `h5py`. Obtain installation tarballs from
   [The h5py project on PyPi](https://pypi.python.org/pypi/h5py).

If you have `open-mpi` installed, to remove it and build `mpich`
instead, use

1. `brew rm open-mpi`
2. `brew cleanup`
3. `brew install mpich`
4. `pip uninstall mpi4py`
5. `pip install --no-binary :all: mpi4py`
6. `brew uninstall hdf5` and re-install per below.

In order to obtain a reasonable build of HDF5 1.10, one should:

* `brew install --build-from-source hdf5 --with-mpi --without-cxx`

### Linux ###

We (Fermilab) use a system called UPS (Unix Product System) for
environment-based setup and use of interdependent packages of known
version and variant. The following packages are available for SLF6
(RHEL6-based), SL7 (RHEL7-based) and Ubuntu 16.04 on
https://scisoft.fnal.gov/:

* hep_hpc (this package)
* GCC 6.3.0
* HDF5 1.10.1 (with and without MPI)
* Python 2.7.13
* mpi4py 2.0.0, numpy1.12.1, six 1.10.0 and h5py 2.7.0

If you are interested in details, please contact us (see below). We are
also in a position to provide binaries for SLF6 and Ubuntu 16.04 on
request using this system.

Use your OS' package manager wherever possible.

## Building the code ##

1. One time per local repository only:  
    ```
    git submodule init
    ```
1. After synchronizing with upstream (including after call to first submodule init):  
    ```
    git submodule update gtest
    ```
1. Make a build directory and `cd` into it.
1. Invoke CMake to configure the code:  
    ```
    CC=<c-compiler> CXX=<c++-compiler> FC=<Fortran-compiler> \
    cmake -DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo> \
    -DCMAKE_INSTALL_PREFIX=<install-area> \
    [-DCMAKE_CXX_STANDARD=<11|14|17>] \
    [-DWANT_MPI=TRUE [-DMPIEXEC_PREFLAGS=...]] \
    [-DWANT_UPS=TRUE] \
    [-DWANT_H5PY=TRUE] \
    <path-to-repository-top-dir>
    ```  
    The `CMakeLists.txt` file includes a safeguard against invoking
    CMake from within the source directory, but you may still have to
    remove some debris if you do this unintentionally.  Define
    `WANT_MPI` appropriately to activate MPI, if it is available and
    desired. Note that your own code may still use MPI even if
    `WANT_MPI` is not set, but the (as yet, very basic) MPI facilities
    of this package will not be available. If the executable for running
    MPI jobs on your system is "srun," then MPI tests will be disabled
    unless you defined MPIEXEC_PREFLAGS (e.g. to set the hardware type,
    time limits, etc.). Define `WANT_UPS` if you wish to build a
    UPS-capable package. Defining `WANT_H5PY` turns on the testing of
    the `concat-h5py` utility (assuming the `h5py` Python package is
    available and compatible with the current compiler, etc.).
    
1. Build the code:   
    ```
    make [-j #]
    ```
1. Run the tests:   
    ```
    ctest [-j #]
    ```
1. Install the code for use:   
    ```
    make install
    ```

N.B. If you wish to update gtest with respect to Google, use a modern git to do the following:  
    ```
    git submodule update --remote --merge gtest
    ```  
    This will cause the index representing the gtest "head" to be
    updated in your local repository. This can be committed if you wish.

## Using the code ##

You will need to instruct your build system to use the headers as
installed and find the libraries. You should also configure your build
system to find the HDF5 and/or MPI libraries, if necessary. Finally, if
you compiled for C++11 compatibility, you should ensure your build
system can find the Boost headers.

The main user-facing classes are `Ntuple` and `Column` -- see the
documentation in each header for details, and `test/hdf5/Ntuple_t.cpp`
for an example of use (installed as `example/Ntuple_t.cpp`). After
running the tests, you may run `h5dump` on `test/hdf5/test-ntuple.h5` to
examine the structure of the data saved.

## Future work ##

The column specification system will be extended to allow user
specification of dataset properties such as chunking and compression.

## Contribution guidelines ##

Please fork the repository and send pull requests.

## Who do I talk to? ##

Send all queries to paterno at fnal dot gov.
