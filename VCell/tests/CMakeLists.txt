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
)

enable_testing()

add_executable(vcell_test ${SRC_FILES} )
target_link_libraries(vcell_test vcell GTest::gtest_main)
#target_link_libraries(TestVCell vcell GTest::gtest_main)
#install(TARGETS TestVCell
#        RUNTIME DESTINATION bin)

include(GoogleTest)
gtest_discover_tests(vcell_test)