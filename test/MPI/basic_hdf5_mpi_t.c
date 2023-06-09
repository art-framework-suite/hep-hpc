/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 *  This example how to work with extendible datasets. The dataset
 *  must be chunked in order to be extendible.
 *
 *  It is used in the HDF5 Tutorial.
 */

/*
  Modifications by Marc Paterno, to use MPI-IO.
*/

#include "hdf5.h"
#include "mpi.h"
#include <stdio.h>

#define FILENAME "extend.h5"
#define DATASETNAME "ExtendibleArray"
#define DATARANK 2

int main(void) {
  // Initialize the MPI environment
  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  printf("Hello from rank: %d\n", my_rank);

  hsize_t dims[2] = {3, 3}; /* dataset dimensions at creation time */
  hsize_t maxdims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};

  /* Create the data space with unlimited dimensions. */
  hid_t dataspace = H5Screate_simple(DATARANK, dims, maxdims);

  hid_t const fprop = H5Pcreate(H5P_FILE_ACCESS);

  herr_t status __attribute__((unused)) =
    H5Pset_fapl_mpio(fprop, MPI_COMM_WORLD, MPI_INFO_NULL);

  /* Create a new file. If file exists its contents will be overwritten. */
  hid_t file = H5Fcreate(FILENAME, H5F_ACC_TRUNC, H5P_DEFAULT, fprop);

  /* Modify dataset creation properties, i.e. enable chunking  */
  hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
  hsize_t chunk_dims[2] = {2, 5};
  status = H5Pset_chunk(prop, DATARANK, chunk_dims);

  /* Create a new dataset within the file using chunk
     creation properties.  */
  hid_t dataset = H5Dcreate2(file, DATASETNAME, H5T_NATIVE_INT, dataspace,
                             H5P_DEFAULT, prop, H5P_DEFAULT);

  int data[3][3] = {{1, 1, 1}, /* data to write */
                    {1, 1, 1},
                    {1, 1, 1}};

  /* Write data to dataset */
  status = H5Dwrite(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

  /* Variables used in extending and writing to the extended portion of dataset
   */
  hsize_t size[2];
  hsize_t offset[2];
  hsize_t dimsext[2] = {7, 3}; /* extend dimensions */
  int dataext[7][3] = {{2, 3, 4}, {2, 3, 4}, {2, 3, 4}, {2, 3, 4},
                       {2, 3, 4}, {2, 3, 4}, {2, 3, 4}};

  /* Extend the dataset. Dataset becomes 10 x 3  */
  size[0] = dims[0] + dimsext[0];
  size[1] = dims[1];
  status = H5Dset_extent(dataset, size);

  /* Select a hyperslab in extended portion of dataset  */
  hid_t filespace = H5Dget_space(dataset);
  offset[0] = 3;
  offset[1] = 0;
  status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, dimsext,
                               NULL);

  /* Define memory space */
  hid_t memspace = H5Screate_simple(DATARANK, dimsext, NULL);

  /* Write the data to the extended portion of dataset  */
  status = H5Dwrite(dataset, H5T_NATIVE_INT, memspace, filespace, H5P_DEFAULT,
                    dataext);

  /* Close resources */
  status = H5Dclose(dataset);
  status = H5Pclose(prop);
  status = H5Sclose(dataspace);
  status = H5Sclose(memspace);
  status = H5Sclose(filespace);
  status = H5Fclose(file);

  MPI_Finalize();
}
