# README #

This code is part of a Fermilab LDRD project studying the use of HDF5 for HEP event data. See the [project wiki](https://bitbucket.org/mpaterno/hdffilestructurestudy/wiki).

### What is this repository for? ###

This code is used for generating files with a variety of organizations of run/subrun/event data.

### How do I get set up? ###

In addition to cloning the repository, you may want to have access to the latest version of the `paterno/pyhdf` Docker image, from Docker Hub (https://hub.docker.com/r/paterno/pyhdf/).

All file generation **except for use of MPI and `mpio`** can be done from within a running Docker container, using the `makefile` provided in this repository. The system has also been tested with a native installation on OSX Yosemite. This is done using:

1. Python from Homebrew.
2. MPI from Homebrew, specifically `mpich`. OpenMPI causes failures in some use of parallel I/O in `h5py`.
3. HDF5 1.10, see below.
4. `mpi4py`, `numpy`, and `six`, installed using `pip`. 
5. `h5py` installed from the installation tarball, after running `python setup.py configure --mpi` to force creation of the MPI-aware version of `h5py`. Obtain installation tarballs from [The h5py project on PyPi](https://pypi.python.org/pypi/h5py).

If you have `open_mpi` installed, to remove it and build `mpich` instead, use

1. `brew rm openmpi`
2. `brew cleanup`
3. `brew install mpich`
4. `pip uninstall mpi4py`
5. `pip install --no-binary :all: mpi4py`
6. `brew uninstall hdf5` and re-install per below.

In order to obtain a reasonable build of HDF5 1.10, one should:

1. `brew tap homebrew/science`
2. `cd $(brew --repository homebrew/science)`
3. download https://bitbucket.org/mpaterno/hdffilestructurestudy/downloads/hdf5fnal.rb and put it into the current directory
4. `brew install --build-from-source hdf5fnal --with-mpi --without-cxx`

To go the Docker route:

1. Install the Docker image using `docker pull`.
2. In the directory to which you cloned this repository, start the image using:  
```
#!bash
$ docker run --rm -it --name hdfwork -v $PWD:/hdfwork -w /hdfwork paterno/pyhdf
```  
To generate files, then just run `make` from within the container (or, if working with native installs, just in a shell). The `makefile` needs to be adjusted to switch between MPI usage and non-MPI usage; look for the invocation of `mpiexec` to see where this is done. In addition, it may be necessary to modify the `make_file.py` (possibly in multiple directories).

### Active branches

The *master* branch of the repository has the code that is using `mpio` to write files.

### Contribution guidelines ###

Please fork the repository and send pull requests.

### Repository architecture and build details ###

The interested observer will notice the presence not only of a `CMakeLists.txt` file and associated `CMakeModules` directory, but also a `GNUmakefie`. The latter manages the in-place invocation of the (mainly python) tests in directories named `ex00`_n_. The `CMakeLists.txt` manages the building and testing of the mainly C and C++ code in the other directories. In order to utilize the 'CMake side' of things, you should:

1. Make a build directory and `cd` into it.
2. Invoke CMake:  
```
CC=<c-compiler> CXX=<c++-compiler> cmake -DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo> <path-to-repository-top-dir>
```  
The `CMakeLists.txt` file includes a safeguard against invoking CMake from within the source directory, but you may still have to remove some debris if you do this unintentionally.

### Who do I talk to? ###

Send all queries to the owner of the repository, paterno at fnal dot gov.