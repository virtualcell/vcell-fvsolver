#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include <VCELL/SolverMain.h>
#include "../build/VCell/tests/testFiles/input/testResourceLocations.h" // provides testResourceLocations


TEST(VCellTest, BasicAssertions) {
    auto retcode_1 = solve(testResourceLocations::EXAMPLE_FV_INPUT, testResourceLocations::EXAMPLE_VCG, testResourceLocations::EXAMPLE_OUTPUT_DIR_1);
    EXPECT_EQ(retcode_1, 0);

    auto retcode_2 = solve(testResourceLocations::EXAMPLE_FV_INPUT, testResourceLocations::EXAMPLE_VCG, testResourceLocations::EXAMPLE_OUTPUT_DIR_2);
    EXPECT_EQ(retcode_2, 0);
}
