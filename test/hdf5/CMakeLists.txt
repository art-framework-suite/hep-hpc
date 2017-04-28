####################################
# Error handling test.
add_executable(errorHandling_t errorHandling_t.cpp)
target_link_libraries(errorHandling_t hep_hpc_hdf5 gtest)
add_test(NAME errorHandling_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/errorHandling_t)

####################################
# Resource test.
add_executable(Resource_t Resource_t.cpp)
target_link_libraries(Resource_t hep_hpc_hdf5 gtest)
add_test(NAME Resource_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/Resource_t)

####################################
# File test.
add_executable(File_t File_t.cpp)
target_link_libraries(File_t hep_hpc_hdf5 gtest)
add_test(NAME File_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/File_t)

####################################
# PropertyList test.
add_executable(PropertyList_t PropertyList_t.cpp)
target_link_libraries(PropertyList_t hep_hpc_hdf5 gtest)
add_test(NAME PropertyList_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/PropertyList_t)

####################################
# Group test.
add_executable(Group_t Group_t.cpp)
target_link_libraries(Group_t hep_hpc_hdf5 gtest)
add_test(NAME Group_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/Group_t)

####################################
# Dataspace test.
add_executable(Dataspace_t Dataspace_t.cpp)
target_link_libraries(Dataspace_t hep_hpc_hdf5 gtest)
add_test(NAME Dataspace_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/Dataspace_t)

####################################
# Dataset test.
add_executable(Dataset_t Dataset_t.cpp)
target_link_libraries(Dataset_t hep_hpc_hdf5 gtest)
add_test(NAME Dataset_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/Dataset_t)

####################################
# Column test.
add_executable(Column_t Column_t.cpp)
add_test(NAME Column_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/Column_t)
target_link_libraries(Column_t hep_hpc_hdf5)
####################################

####################################
# Ntuple test.
add_executable(Ntuple_t Ntuple_t.cpp)
add_test(NAME Ntuple_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/Ntuple_t)
target_link_libraries(Ntuple_t hep_hpc_hdf5)
####################################