# README #

This code is part of a Fermilab LDRD project studying the use of HDF5 for HEP event data. See the [project wiki](https://bitbucket.org/mpaterno/hdffilestructurestudy/wiki).

### What is this repository for? ###

This code is used for generating files with a variety of organizations of run/subrun/event data.

### How do I get set up? ###

In addition to cloning the repository, you may want to have access to the latest version of the `paterno/pyhdf` Docker image, from Docker Hub (https://hub.docker.com/r/paterno/pyhdf/).

All file generation **except for use of MPI and `mpio`** can be done from within a running Docker container, using the `makefile` provided in this repository. The system has also been tested with a native installation on OSX Yosemite. This is done using:

1. Python from Homebrew.
2. MPI from Homebrew, specifically `open-mpi`.
3. HDF5 from Homebrew, built from source with the options `--with-mpi --without-cxx`.
4. `mpi4py` (version 2.0.0) installed using `pip`.
5. `h5py` installed from the installation tarball, after running `python setup.py configure --mpi` to force creation of the MPI-aware version of `h5py`. Obtain installation tarballs from [The h5py project on PyPi](https://pypi.python.org/pypi/h5py).

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
The *no_mpio* branch of the repository has the code using the default I/O driver.

### Contribution guidelines ###

Please fork the repository and send pull requests.

### Who do I talk to? ###

Send all queries to the owner of the repository, paterno at fnal dot gov.