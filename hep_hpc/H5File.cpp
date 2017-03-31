#include "hep_hpc/H5File.hpp"
#include "hep_hpc/H5Exception.hpp"

hep_hpc::H5File::H5File(hid_t file)
  :
  h5file_([file](){return file;}, [](hid_t){})
{
}

hep_hpc::H5File::H5File(std::string const & filename,
                         unsigned int const flag,
                         H5PropertyList const fileCreationProperties,
                         H5PropertyList const fileAccessProperties)
  :
  h5file_([&]()
          { HID_t result;
            if (flag == H5F_ACC_RDONLY || flag == H5F_ACC_RDWR) {
              // Open.
              if (fileCreationProperties) { // ERROR.
                throw
                  H5Exception("H5File(): flags indicated file-open semantics, "
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
