/***********************************************************************
* test-get-class-name
*
* Verify correct cleanup of the result of H5Pget_class_name().
*
***********************************************************************/

#include "hdf5.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
  /* Create a new property list. */
  hid_t const fprop = H5Pcreate(H5P_FILE_ACCESS);
  assert(fprop > 0);
  hid_t const classID = H5Pget_class(fprop);
  assert(classID > 0);
  char * buf = H5Pget_class_name(classID);
  assert(buf != NULL);
  printf("%s\n", buf);
/*  free(buf); */
  H5free_memory(buf);
  H5Pclose(fprop);
  return 0;
}
