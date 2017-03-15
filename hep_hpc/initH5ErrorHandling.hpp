#ifndef hep_hpc_initH5ErrorHandling_hpp
#define hep_hpc_initH5ErrorHandling_hpp
////////////////////////////////////////////////////////////////////////
// initH5ErrorHandling()
//
// This will cause HDF5 functions in the current thread to throw an
// exception when they would otherwise print an error message and return
// a non-SUCCESS code.
//
// N.B. The implementation of this functionality requires POSIX.1-2008
// support, specifically the function open_memstream(). If support for
// other platforms is required, consider using a third party
// implementation such as http://piumarta.com/software/memstream/,
//
// N.M.B. It is unclear whether this functionality conflicts with the
// note regarding callbacks in the documentation of the H5Eset_auto()
// HDF5 function. See
// https://support.hdfgroup.org/HDF5/doc1.8/RM/RM_H5E.html#Error-SetAuto
// for more details.
////////////////////////////////////////////////////////////////////////

namespace hep_hpc {
  void initH5ErrorHandling();
}


#endif /* hep_hpc_initH5ErrorHandling_hpp */
