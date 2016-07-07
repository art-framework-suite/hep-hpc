# README #

This code is part of a Fermilab LDRD project studying the use of HDF5 for HEP event data.

### What is this repository for? ###

This code is used for generating files with a variety of organizations of run/subrun/event data.

### How do I get set up? ###

In addition to cloning the repository, you'll need to have access to the latest version of the `paterno/pyhdf` Docker image, from Docker Hub (https://hub.docker.com/r/paterno/pyhdf/).

All file generation is done from within a running Docker container, using the `makefile` provided in this repository.

1. Install the Docker image using `docker pull`.
2. In the directory to which you cloned this repository, start the image using:

```
#!bash

$ docker run --rm -it --name hdfwork -v $PWD:/hdfwork -w /hdfwork paterno/pyhdf
```

To generate files, then just run `make` from within the container.

### Contribution guidelines ###

Please fork the repository and send pull requests.

### Who do I talk to? ###

Send all queries to the owner of the repository, paterno at fnal dot gov.