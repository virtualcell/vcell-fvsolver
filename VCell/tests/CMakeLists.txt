cmake_minimum_required (VERSION 3.22)
project(VCellTest CXX)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG          v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if (NOT WINDOWS)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

set(SRC_FILES
        TestVCell.cpp
        TestSmoldyn.cpp
        testFiles/SmoldynComplexDataStructures/TestSmoldynCHashtable.cpp
        testFiles/SmoldynComplexDataStructures/TestSmoldynCLinkedList.cpp
        testFiles/SmoldynComplexDataStructures/TestSmoldynCVector.cpp
)

#####
# Build Testing File Locations Dynamically
#
# Only the ROOT is baked in; testResourceLocations.h composes the individual
# paths from it at run time and honours $VCELL_TEST_RESOURCES as an override,
# so vcell_test can run against a relocated copy of testFiles/ on a machine
# that has no source tree (see make-test-bundle.sh).
get_filename_component(TEST_RESOURCES_ROOT testFiles ABSOLUTE)
configure_file(testFiles/input/testResourceLocations.h.in testFiles/input/testResourceLocations.h @ONLY)
#####

enable_testing()

add_executable(vcell_test ${SRC_FILES} )
target_include_directories(vcell_test PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
message("Included Directory: ${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(vcell_test vcell GTest::gtest_main)
#target_link_libraries(TestVCell vcell GTest::gtest_main)
#install(TARGETS TestVCell
#        RUNTIME DESTINATION bin)

include(GoogleTest)
gtest_discover_tests(vcell_test)
# copy tests files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/testFiles DESTINATION ${CMAKE_CURRENT_BINARY_DIR})