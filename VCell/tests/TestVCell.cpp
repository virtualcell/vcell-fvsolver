#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstring>  // Note the c++ variant, not the .h original
#include <cstdlib>  // Note the c++ variant, not the .h original

#include <VCELL/SolverMain.h>

#include "smoldynfuncs.h"
#include "../build/VCell/tests/testFiles/input/testResourceLocations.h" // provides testResourceLocations


TEST(VCellTest, ExcerciseFV) {
    auto retcode_1 = solve(testResourceLocations::EXAMPLE_FV_INPUT, testResourceLocations::EXAMPLE_VCG, testResourceLocations::EXAMPLE_OUTPUT_DIR_1);
    EXPECT_EQ(retcode_1, 0);

    auto retcode_2 = solve(testResourceLocations::EXAMPLE_FV_INPUT, testResourceLocations::EXAMPLE_VCG, testResourceLocations::EXAMPLE_OUTPUT_DIR_2);
    EXPECT_EQ(retcode_2, 0);
}

TEST(VCellTest, ExerciseSmoldyn) {
    std::string first_arg{"ignored"};
    std::string second_arg{testResourceLocations::EXAMPLE_SMOLDYN_INPUT};
    // Allocate zero-initialized memory
    char* first_arg_ptr = static_cast<char*>(std::calloc(first_arg.size() + 1, sizeof(char)));
    char* second_arg_ptr = static_cast<char*>(std::calloc(second_arg.size() + 1, sizeof(char)));
    ASSERT_TRUE(first_arg_ptr && second_arg_ptr) << "Memory allocation failed";

    std::strcpy(first_arg_ptr, first_arg.c_str());
    std::strcpy(second_arg_ptr, second_arg.c_str());
    char* arg_array[2];
    arg_array[0] = first_arg_ptr;
    arg_array[1] = second_arg_ptr;

    int retcode_1 = runSmoldyn(2, arg_array);
    EXPECT_EQ(retcode_1, 0);
    int retcode_2 = runSmoldyn(2, arg_array);
    EXPECT_EQ(retcode_2, 0);

    std::free(first_arg_ptr);
    std::free(second_arg_ptr);
}


