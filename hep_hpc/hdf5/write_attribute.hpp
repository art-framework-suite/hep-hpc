#include "hdf5.h"

#include <string>
#include <vector>

namespace hep_hpc {
  namespace hdf5 {
    herr_t write_attribute(hid_t groupOrDataset,
                           std::string const & name,
                           std::string const & value);
    
    herr_t write_attribute(hid_t groupOrDataset,
			   std::string const & name,
			   std::vector<std::string> const & values);
  }
}
