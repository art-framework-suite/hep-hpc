#include "hep_hpc/H5File.hpp"

hep_hpc::H5File::H5File(std::string const & filename,
                         unsigned int flags,
                         H5PropertyList fileCreationProperties,
                         H5PropertyList fileAccessProperties)
  :
  h5file_([&]()
          { return H5Fcreate(filename.c_str(),
                             static_cast<unsigned>(flags),
                             fileCreationProperties,
                             fileAccessProperties); },
          &H5Fclose)
{
}
