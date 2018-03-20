/***********************************************************************
* test-no-write-ranks
*
* Demonstrate what happens with collective writes when a rank has
* nothing to write.
*
***********************************************************************/

#include "mpi.h"
#include "hdf5.h"

#include <stdio.h>
#include <stdlib.h>

static int n_ranks;
static int my_rank;

int main(int argc, char **argv)
{
  /* Initialize MPI. */
  int const required = MPI_THREAD_FUNNELED;
  int provided;
  herr_t status = MPI_Init_thread(&argc, &argv, required, &provided);
  if (status != MPI_SUCCESS || required != provided) {
    fprintf(stderr, "ERROR: unable to initialize MPI.\n");
    exit(2);
  }

  MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  hsize_t const chunk_size = 7ull;
  hsize_t const n_chunks = n_ranks;

  /* Create an N x 2 x 3 dataset. */
  hsize_t const dims[3] = {chunk_size * n_chunks, 2, 3};
  hsize_t const maxdims[3] = {H5S_UNLIMITED, 2, 3};
  hsize_t const chunk_dims[3] = {chunk_size, 2, 3};

  /* Create a dataspace. */
  hid_t const dataspace = H5Screate_simple(3, dims, maxdims);

  /* Create a new file (overwrite if it already exists). */
  hid_t const fprop = H5Pcreate(H5P_FILE_ACCESS);
  status = H5Pset_fapl_mpio(fprop, MPI_COMM_WORLD, MPI_INFO_NULL);
  hid_t const file =
    H5Fcreate("test-no-write-ranks.hdf5",
              H5F_ACC_TRUNC,
              H5P_DEFAULT,
              fprop);

  /* Enable chunking, compression. */
  hid_t const prop = H5Pcreate(H5P_DATASET_CREATE);
  status = H5Pset_chunk(prop, 3, chunk_dims);
  status = H5Pset_deflate(prop, 6);

  /* Create dataset. */
  hid_t const dataset =
    H5Dcreate2(file, "dset", H5T_NATIVE_INT, dataspace,
               H5P_DEFAULT, prop, H5P_DEFAULT);

  int * const data = malloc(chunk_size * 2 * 3 * sizeof(int));

  for (hsize_t i = 0; i < chunk_size; ++i) {
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 3; ++k) {
        data[(i * 2 * 3) + (j * 3) + k] =
          my_rank * 1000 + i * 100 + j * 10 + k;
      }
    }
  }

  hid_t mem_dspace;

  if (my_rank == 1) { /* Pretend we have nothing to write here. */
    H5Sselect_none(dataspace);
    hsize_t const shape[3] = { 0, 0, 0 };
    mem_dspace = H5Screate_simple(3, shape, shape);
  } else {
    hsize_t const n_elements[3] = {chunk_size, 2, 3};
    hsize_t const offsets[3] = {my_rank * chunk_size, 0, 0};
    hsize_t const block_count[3] = {1, 1, 1};
    H5Sselect_hyperslab(dataspace,
                        H5S_SELECT_SET,
                        offsets,
                        NULL,
                        block_count,
                        n_elements);
    mem_dspace = H5Screate_simple(3, n_elements,  n_elements);
  }

  hid_t const xprop = H5Pcreate(H5P_DATASET_XFER);
  status = H5Pset_dxpl_mpio(xprop, H5FD_MPIO_COLLECTIVE);

  status = H5Dwrite(dataset,
                    H5T_NATIVE_INT,
                    mem_dspace,
                    dataspace,
                    xprop,
                    data);

  status = H5Dclose(dataset);
  status = H5Sclose(dataspace);
  status = H5Sclose(mem_dspace);
  status = H5Pclose(fprop);
  status = H5Pclose(prop);
  status = H5Pclose(xprop);
  status = H5Fclose(file);

  MPI_Finalize();
  free(data);
  return status;
}
