#include <gtest/gtest.h>

#include <VCELL/SolverMain.h>

TEST(VCellTest, BasicAssertions) {
    std::string inputDir = "../../testFiles/input/";
    std::string outputDir = "../../testFiles/output/";

    std::string fvInputFile = inputDir + "SimID_11538992_0_.fvinput";
    std::string vcgInputFile = inputDir + "SimID_11538992_0_.vcg";
    std::string testOutputDir_1 = outputDir + "test_output_1";
    std::string testOutputDir_2 = outputDir + "test_output_2";

    auto retcode_1 = solve(fvInputFile, vcgInputFile, testOutputDir_1);
    EXPECT_EQ(retcode_1, 0);

    auto retcode_2 = solve(fvInputFile, vcgInputFile, testOutputDir_2);
    EXPECT_EQ(retcode_2, 0);
 }

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
