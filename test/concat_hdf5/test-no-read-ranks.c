/***********************************************************************
* test-no-read-ranks
*
* Demonstrate what happens with collective reads when a rank has
* nothing to write.
*
* Requires a file written by test-no-write-ranks.c.
*
***********************************************************************/

#include "mpi.h"
#include "hdf5.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hep_hpc/hdf5/detail/hdf5_compat.h"

static int n_ranks;
static int my_rank;

static bool want_visit = false;

static
herr_t
visit_item(hid_t root_id,
           char const * obj_name,
           H5O_info_t const * obj_info,
           void * data)
{
  // Find the dataset we want and open it.
  if (strcmp("dset", obj_name) == 0) {
    /* Open the dataset. */
    *(hid_t *)data = HEP_HPC_OPEN_BY(root_id, obj_info->HEP_HPC_ADDR_OR_TOKEN);
  }
  return 0;
}

int main(int argc, char **argv)
{
  /* Initialize MPI */
  int const required = MPI_THREAD_FUNNELED;
  int provided;
  herr_t status = MPI_Init_thread(&argc, &argv, required, &provided);
  if (status != MPI_SUCCESS || required != provided) {
    fprintf(stderr, "ERROR: unable to initialize MPI.\n");
    exit(2);
  }

  if (argc == 2 && strcmp(argv[1], "--visit") == 0) {
    printf("Selected visitor operation.\n");
    want_visit = true;
  }

  MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  hsize_t const chunk_size = 7ull;

  /* Open input file as created by test-no-write-ranks.c. */
  hid_t fprop = H5Pcreate(H5P_FILE_ACCESS);
  status = H5Pset_fapl_mpio(fprop, MPI_COMM_WORLD, MPI_INFO_NULL);
  hid_t file = H5Fopen("test-no-write-ranks.hdf5", H5F_ACC_RDONLY, fprop);

  /* Open the dataset, one way or the other. */
  hid_t in_dataset = -1;
  if (want_visit) {
    H5Ovisit(file, H5_INDEX_NAME, H5_ITER_NATIVE, &visit_item, &in_dataset
#if H5_VERSION_GE(1,12,0)
             ,H5O_INFO_BASIC
#endif
            );
  } else {
    in_dataset = H5Dopen2(file, "dset", H5P_DEFAULT);
  }
  if (in_dataset < 0) {
    fprintf(stderr, "Unable to find / open expected dataset \"dset\".\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  /* To hold incoming data. */
  int * data = calloc(chunk_size * 2 * 3, sizeof(int));

  hid_t in_dataspace = H5Dget_space(in_dataset);
  hid_t mem_dspace;

  hsize_t const ndims = H5Sget_simple_extent_ndims(in_dataspace);
  if (ndims != 3) {
    fprintf(stderr, "ERROR: dset has unexpected rank!\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  } 
  hsize_t const n_elements_to_read[3] = {chunk_size, 2, 3};
  hsize_t n_elements[3], dummy[3];
  status =
    H5Sget_simple_extent_dims(in_dataspace, n_elements, dummy);
  if (memcmp(n_elements_to_read + 1, n_elements + 1, 2 * sizeof(hsize_t)) != 0) {
    fprintf(stderr, "ERROR: dset has unexpected extents: (%llu, %llu %llu)!\n",
            n_elements[0], n_elements[1], n_elements[2]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  if (my_rank == 1 || n_elements[0] <= my_rank * chunk_size) { /* Don't read anything here. */
    printf("NOP read for rank %d\n", my_rank);
    H5Sselect_none(in_dataspace);
    mem_dspace = H5Screate(H5S_NULL);
  } else {
    hsize_t offsets[3] = {my_rank * chunk_size, 0, 0};
    hsize_t block_count[3] = {1, 1, 1};
    H5Sselect_hyperslab(in_dataspace,
                        H5S_SELECT_SET,
                        offsets,
                        NULL,
                        block_count,
                        n_elements_to_read);
    mem_dspace = H5Screate_simple(3, n_elements_to_read,  n_elements_to_read);
  }

  /* Set up collective read/write operations. */
  hid_t xprop = H5Pcreate(H5P_DATASET_XFER);
  status = H5Pset_dxpl_mpio(xprop, H5FD_MPIO_COLLECTIVE);

  /* Read the data. */
  status = H5Dread(in_dataset,
                   H5T_NATIVE_INT,
                   mem_dspace,
                   in_dataspace,
                   xprop,
                   data);

  status = H5Dclose(in_dataset);
  status = H5Sclose(in_dataspace);
  status = H5Sclose(mem_dspace);
  status = H5Pclose(fprop);
  status = H5Pclose(xprop);
  status = H5Fclose(file);

  MPI_Finalize();
  free(data);
  return status;
}
