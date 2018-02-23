#include "hep_hpc/hdf5/File.hpp"
#include "hep_hpc/hdf5/Exception.hpp"

hep_hpc::hdf5::File::File(std::string const & filename,
                           unsigned int const flag,
                           PropertyList fileCreationProperties,
                           PropertyList fileAccessProperties)
  :
  h5file_([&]()
          { HID_t result;
            if (flag == H5F_ACC_RDONLY || flag == H5F_ACC_RDWR) {
              // Open.
              if (fileCreationProperties) { // ERROR.
                throw
                  Exception("File(): flags indicated file-open semantics, "
                            "but non-trivial creation properties specified!");
              }
              result = H5Fopen(filename.c_str(),
                               flag,
                               fileAccessProperties);
            } else {
              // Create.
              result = H5Fcreate(filename.c_str(),
                                 flag,
                                 fileCreationProperties,
                                 fileAccessProperties);
            }
            return result;
          }, &H5Fclose)
{
}
