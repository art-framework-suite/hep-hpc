#include "hep_hpc/H5File.hpp"

namespace {
  auto
  h5FileCreator =
    [](std::string const & filename,
       unsigned int flags) { return H5Fcreate(filename.c_str(),
                                              static_cast<unsigned>(flags),
                                              H5P_DEFAULT,
                                              H5P_DEFAULT); };
}

hep_hpc::H5File::H5File(std::string const & filename, unsigned int flags)
  :
  h5file_(h5FileCreator, &H5Fclose, filename, flags)
{
}
