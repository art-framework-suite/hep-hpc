#include "hep_hpc/hdf5/HID_t.hpp"
#include "hep_hpc/hdf5/Resource.hpp"
#include "hep_hpc/hdf5/errorHandling.hpp"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"
#include "hdf5.h"

#include <string>

using namespace hep_hpc::hdf5;

namespace {
  class ErrorHandling : public ::testing::Test {
protected:
    void SetUp() override
      {
        ErrorController::resetErrorHandler();
      }
  };
} 

TEST_F(ErrorHandling, currentMode)
{
  ASSERT_EQ(ErrorController::currentMode(), ErrorMode::HDF5_DEFAULT);
}

TEST_F(ErrorHandling, setErrorHandler_mode_NONE)
{
  auto const eh = detail::saveErrorHandler();
  ASSERT_EQ(eh.mode, ErrorMode::HDF5_DEFAULT);
  ErrorController::setErrorHandler(ErrorMode::NONE);
  ASSERT_EQ(ErrorController::currentMode(), ErrorMode::NONE);
  HID_t const bad_hid;
  ASSERT_EQ(ErrorController::call(&H5Fclose, bad_hid), -1ll);
  SUCCEED();
}

TEST_F(ErrorHandling, setErrorHandler_mode_HDF5_DEFAULT)
{ 
  ErrorController::setErrorHandler(ErrorMode::NONE);
  auto eh = detail::saveErrorHandler();
  ASSERT_EQ(eh.mode, ErrorMode::NONE);
  ErrorController::setErrorHandler(ErrorMode::HDF5_DEFAULT);
  ASSERT_EQ(ErrorController::currentMode(), ErrorMode::HDF5_DEFAULT);
  HID_t const bad_hid;
  ASSERT_EQ(ErrorController::call(&H5Fclose, bad_hid), -1ll);
  SUCCEED();
}

TEST_F(ErrorHandling, setErrorHandler_mode_CUSTOM)
{
  ASSERT_THROW(ErrorController::setErrorHandler(ErrorMode::CUSTOM),
               std::logic_error);
}

TEST_F(ErrorHandling, setErrorHandler_mode_EXCEPTION)
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  HID_t const bad_hid;
  ASSERT_THROW(ErrorController::call(&H5Fclose, bad_hid),
               hep_hpc::hdf5::Exception);
}

namespace {
  std::string ref_string = "Error function called successfully.";
  herr_t errFunc(hid_t, std::string * const sPtr)
  {
    using namespace std::string_literals;
    *sPtr = ref_string;
    return 0;
  }
}

TEST_F(ErrorHandling, setErrorHandling_func_1)
{
  std::string err;
  ErrorController::setErrorHandler((H5E_auto2_t) &errFunc, &err);
  ASSERT_EQ(ErrorController::currentMode(), ErrorMode::CUSTOM);
  HID_t const bad_hid;
  ASSERT_EQ(ErrorController::call(&H5Fclose, bad_hid), -1ll);
  ASSERT_EQ(err, ref_string);
}

TEST_F(ErrorHandling, setErrorHandling_func_2)
{
  std::string err;
  ErrorController::setErrorHandler((H5E_auto2_t) &errFunc, &err);
  ASSERT_EQ(ErrorController::currentMode(), ErrorMode::CUSTOM);
  ErrorController::setErrorHandler(nullptr, nullptr);
  ASSERT_EQ(ErrorController::currentMode(), ErrorMode::NONE);
  HID_t const bad_hid;
  ASSERT_EQ(ErrorController::call(&H5Fclose, bad_hid), -1ll);
  ASSERT_TRUE(err.empty());
}

TEST_F(ErrorHandling, setAndSaveErrorHandler_mode)
{
  ErrorController::setErrorHandler(ErrorMode::NONE);
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::NONE, nullptr, nullptr }));
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::EXCEPTION, nullptr, nullptr }));
}

TEST_F(ErrorHandling, setAndSaveErrorHandler_func_1)
{
  ErrorController::setErrorHandler(ErrorMode::NONE);
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::NONE, nullptr, nullptr }));
  std::string err;
  ErrorController::setErrorHandler((H5E_auto2_t) &errFunc, &err);
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::CUSTOM, (H5E_auto2_t) &errFunc, &err }));
}

TEST_F(ErrorHandling, setAndSaveErrorHandler_func_2)
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::EXCEPTION, nullptr, nullptr }));
  std::string err;
  ErrorController::setErrorHandler(nullptr, nullptr);
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::NONE, nullptr, nullptr }));
}

TEST_F(ErrorHandling, resetErrorHandler)
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  ASSERT_EQ(ErrorController::currentMode(), ErrorMode::EXCEPTION);
  ErrorController::resetErrorHandler(); 
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::HDF5_DEFAULT, (H5E_auto2_t) &H5Eprint, stderr })); 
}

TEST_F(ErrorHandling, save_and_restoreErrorHandler)
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  ASSERT_EQ(ErrorController::currentMode(), ErrorMode::EXCEPTION);
  ErrorController::saveErrorHandler();
  ErrorController::setErrorHandler(ErrorMode::NONE);
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::NONE, nullptr, nullptr }));
  ErrorController::restoreErrorHandler(); 
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::EXCEPTION, nullptr, nullptr }));
}

TEST_F(ErrorHandling, ScopedErrorHandler)
{
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::HDF5_DEFAULT, (H5E_auto2_t) &H5Eprint, stderr }));
  {
    ScopedErrorHandler seh(ErrorMode::EXCEPTION);
    ASSERT_EQ(detail::saveErrorHandler(),
              (detail::StoredErrorHandler { ErrorMode::EXCEPTION, nullptr, nullptr }));
  }
  ASSERT_EQ(detail::saveErrorHandler(),
            (detail::StoredErrorHandler { ErrorMode::HDF5_DEFAULT, (H5E_auto2_t) &H5Eprint, stderr }));
}

TEST_F(ErrorHandling, call_raw)
{
  HID_t const bad_hid;
  ErrorController::setErrorHandler();
  ASSERT_EQ(ErrorController::call(&H5Fclose, bad_hid), -1ll);  
}

TEST_F(ErrorHandling, call_mode)
{
  HID_t const bad_hid;
  ASSERT_EQ(ErrorController::call(ErrorMode::NONE, &H5Fclose, bad_hid), -1ll);  
}

TEST_F(ErrorHandling, call_func)
{
  HID_t const bad_hid;
  std::string err;
  ASSERT_EQ(ErrorController::call((H5E_auto2_t)&errFunc, &err, &H5Fclose, bad_hid), -1ll);  
  ASSERT_EQ(err, ref_string);
}

TEST_F(ErrorHandling, nested_errors)
{
  ErrorController::setErrorHandler(ErrorMode::EXCEPTION);
  try {
    Resource<hid_t> r1((hid_t)1101, &H5Fclose);
    {
      Resource<hid_t> r2((hid_t)1102, &H5Fclose);
    }
    FAIL();
  }
  catch (::hep_hpc::hdf5::Exception const & e) {
    SUCCEED();
  }
  catch (...)
  {
    FAIL();
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
