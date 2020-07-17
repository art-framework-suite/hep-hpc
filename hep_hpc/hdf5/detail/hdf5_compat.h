#ifndef HEP_HPC_HDF5_COMPAT_H
#define HEP_HPC_HDF5_COMPAT_H
/*
  hdf5_compat.h

  Macros to aid accommodation of different versions of HDF5.
*/

#ifdef __cplusplus
extern "C" {
#endif
#include "H5public.h"
#ifdef __cplusplus
}
#endif

// Handle API change for HDF5 1.12.0 (H5O_info1_t -> H5O_info2_t).
#if H5_VERSION_GE(1,12,0)
#define HEP_HPC_ADDR_OR_TOKEN token
#define HEP_HPC_OPEN_BY H5Oopen_by_token
#else
#define HEP_HPC_ADDR_OR_TOKEN addr
#define HEP_HPC_OPEN_BY H5Oopen_by_addr
#endif

#endif /* HEP_HPC_HDF5_COMPAT_H */
