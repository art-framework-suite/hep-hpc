#include "hep_hpc/H5File.hpp"

hep_hpc::H5File::H5File(std::string const & filename,
                         unsigned int flags,
                         hid_t fcpl_id,
                         hid_t fapl_id)
  :
  h5file_([&]()
          { return H5Fcreate(filename.c_str(),
                             static_cast<unsigned>(flags),
                             fcpl_id,
                             fapl_id); },
          &H5Fclose)
{
}
