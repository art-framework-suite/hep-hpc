/***********************************************************************
* test-resize
*
* Open collective datasets in a file and keep resizing them.
*
***********************************************************************/
#define _POSIX_C_SOURCE 200809L /* Required for strdup on Linux. */

#include "mpi.h"
#include "hdf5.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int n_ranks;
static int my_rank;

static hsize_t const max_rows = 100000000;
static hsize_t const rows_per_resize = 48570;
static hsize_t const chunk_size = 64ull;

static int calc_n_ranks_chars() {
  int result = 1;
  int tmp = n_ranks;
  while ((tmp /= 10) > 0) {
    result += 1;
  }
  return result;
}

static char * rank_report = NULL;
static int n_dsets = 10;

static void report(char *msg)
{
  fprintf(stderr, "%s %s\n", rank_report, msg);
}

typedef struct
{
  hid_t dset;
  char * name;
  hsize_t ndims;
  hsize_t * shape;
} ds_info;

static void clear_info(ds_info * info) 
{
  if (info != NULL) {
    free(info->shape);
    free(info->name);
    H5Dclose(info->dset);
  }
}

static
void
init_dset(hid_t file,
          ds_info * info,
          char * name,
          hsize_t ndims,
          hsize_t shape[])
{
  char msg[1024];
  snprintf(msg, 1024, "Preparing to create dataset %s.", name);
  report(msg);
  if (! (info->dset < 0)) {
    clear_info(info);
  }
  info->name = strdup(name);
  info->shape = calloc(ndims, sizeof(hsize_t));
  hsize_t * maxdims = calloc(ndims, sizeof(hsize_t));
  hsize_t * chunkdims = calloc(ndims, sizeof(hsize_t));
  memcpy(info->shape, shape, ndims * sizeof(hsize_t));
  memcpy(maxdims + 1, shape + 1, (ndims - 1) * sizeof(hsize_t));
  maxdims[0] = H5S_UNLIMITED;
  memcpy(chunkdims + 1, shape + 1, (ndims - 1) * sizeof(hsize_t));
  chunkdims[0] = chunk_size;
  hid_t dataspace = H5Screate_simple(ndims, info->shape, maxdims);
  hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(prop, ndims, chunkdims);
  H5Pset_deflate(prop, 6);
  snprintf(msg, 1024, "Creating %lli-D dataset %s with %lli rows.",
           ndims, name, info->shape[0]);
  report(msg);
  info->dset = H5Dcreate2(file, name, H5T_NATIVE_USHORT, dataspace,
                          H5P_DEFAULT, prop, H5P_DEFAULT);
  snprintf(msg, 1024, "Created %lli-D dataset %s with %lli rows.",
           ndims, name, info->shape[0]);
  report(msg);
  H5Pclose(prop);
  H5Sclose(dataspace);
  free(maxdims);
  free(chunkdims);
}

static
hsize_t
resize_datasets(ds_info * dsets, int n_dsets)
{
  hsize_t ds_size = dsets[0].shape[0];
  for (int i = 0; i < n_dsets; ++i) {
    dsets[i].shape[0] += rows_per_resize;
    if (i == 0) {
      ds_size = dsets[i].shape[0];
    }
    char buf[1024];
    snprintf(buf,
             1024,
             "Resizing dataset %s to %lli rows.",
             dsets[i].name,
             dsets[i].shape[0]);
    report(buf);
    H5Dset_extent(dsets[i].dset, dsets[i].shape);    
  }
  return ds_size;
}

/* Main program. */
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

  /* Initialize MPI info. */
  MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  /* Initialize the rank report header. */
  int const n_ranks_chars = calc_n_ranks_chars();
  int const report_size = n_ranks_chars * 2 + 4;
  rank_report = malloc(report_size);
  char fmt[24];
  snprintf(fmt, 24, "[%%0%ii/%i]", n_ranks_chars, n_ranks);
  snprintf(rank_report, report_size, fmt, my_rank);

  report("Initialized.");

  /* Create and clear dataset info. */
  ds_info dsets[n_dsets];
  memset(dsets, 0, n_dsets * sizeof(ds_info));
  for (int i = 0; i < n_dsets; ++i) {
    dsets[i].dset = -1;
  }
  
  /* Create a new file (overwrite if it already exists). */
  hid_t fprop = H5Pcreate(H5P_FILE_ACCESS);
  status = H5Pset_fapl_mpio(fprop, MPI_COMM_WORLD, MPI_INFO_NULL);
  hid_t file = H5Fcreate("test-resize.hdf5", H5F_ACC_TRUNC, H5P_DEFAULT, fprop);

  /* Create datasets. */
  hsize_t dims1[3] = {rows_per_resize, 240, 3072};
  hsize_t dims2[2] = {rows_per_resize, 240};

  init_dset(file, dsets, "ds00", 3, dims1);
  for (int i = 1; i < n_dsets; ++i) {
    char dsname[20];
    snprintf(dsname, 20, "ds%02i", i);
    init_dset(file, dsets + i, dsname, 2, dims2);
  }

  /* Resize the datasets in turn. */
  while (resize_datasets(dsets, n_dsets) <= max_rows);

  /* Clean up all datasets. */
  for (int i = 0; i < n_dsets; ++i) {
    clear_info(dsets + i);
  }
  
  status = H5Pclose(fprop);
  status = H5Fclose(file);

  MPI_Finalize();
  free(rank_report);
  return status;
}
