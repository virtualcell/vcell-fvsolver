#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include <VCELL/SolverMain.h>
#include <ComplexDataStructures/Hashtable.h>
#include <ComplexDataStructures/Vector.h>
#include <ComplexDataStructures/LinkedList.h>

static std::string getVectorAsSingleString(Vector* list);
static std::string getLinkedListAsSingleString(LinkedList* list);

TEST(VCellTest, BasicAssertions) {
    std::filesystem::path inputDir = "testFiles/input/BasicAssertions";
    std::filesystem::path outputDir = "testFiles/output/BasicAssertions";

    std::filesystem::path fvInputFile = inputDir / "SimID_11538992_0_.fvinput";
    std::filesystem::path vcgInputFile = inputDir / "SimID_11538992_0_.vcg";
    std::string testOutputDir_1 = outputDir / "test_output_1";
    std::string testOutputDir_2 = outputDir / "test_output_2";

    auto retcode_1 = solve(fvInputFile, vcgInputFile, testOutputDir_1);
    EXPECT_EQ(retcode_1, 0);

    auto retcode_2 = solve(fvInputFile, vcgInputFile, testOutputDir_2);
    EXPECT_EQ(retcode_2, 0);
}

TEST(VCellTest, CustomCHashtableTest) {
    Hashtable* counts = createNewHashtable();
    
    if (counts == nullptr) {fprintf(stderr, "Hashtable did not get allocated!\n"); FAIL();}

    std::filesystem::path currentDir = std::filesystem::current_path().parent_path().parent_path() / "VCell" / "tests";
    if (!exists(currentDir)) {fprintf(stderr, "Relevant parent directory doesn't exist; have path names changed?\n"); FAIL();}
    std::filesystem::path inputDir = currentDir / "testFiles/input/CustomCHashtableTest";
    std::filesystem::path inputFile = inputDir / "testFile.txt";
    if (!exists(inputFile)) {fprintf(stderr, "Relevant input doesn't exist; have path names changed?\n"); FAIL();}
    std::ifstream inputStream{}; inputStream.open(inputFile.string());
    if (!inputStream.is_open()) {fprintf(stderr, "Unable to open file\n"); FAIL();}
    std::string line{};
    while (std::getline(inputStream, line)) {
        if (counts->contains(counts, line.c_str())) {
            void* value = counts->getFrom(counts, line.c_str());
            // Already exists, increment int that value points to.
            int* pcount = (int*)value;
            (*pcount)++;
            continue;
        }
        // Word not found, allocate space for new int and set to 1.
        int* pcount = (int*)malloc(sizeof(int));
        if (pcount == nullptr) {fprintf(stderr, "Memory allocation of count-value failed!\n"); FAIL();}
        *pcount = 1;
        if (counts->putIn(counts, line.c_str(), pcount) == NULL) {fprintf(stderr, "Unable to put new counts in hashtable!\n"); FAIL();}
    }
    inputStream.close();

    // Spot Test
    if (!counts->contains(counts, "pizzas")) {fprintf(stderr, "Hashtable does not contain `pizzas`!\n"); FAIL();}
    if (2 != *(int*)counts->getFrom(counts, "pizzas")) {fprintf(stderr, "Hashtable contains incorrect number of `pizzas`!\n"); FAIL();}
    if (counts->contains(counts, "hackathons")) {fprintf(stderr, "Hashtable contains `hackathons`, when it shouldn't!\n"); FAIL();}
    errno = 0;
    if (counts->getFrom(counts, "hackathons") != NULL || errno == 0) {fprintf(stderr, "Bad Access did not properly error out!\n"); FAIL();}

    // Iterator test
    HashtableIterator it = counts->getIterator(counts);
    int counter;
    for (counter = 0; it.iter(&it); counter++) {
        if (!counts->contains(counts, it.key)) {fprintf(stderr, "Iterator does not produce valid keys!\n"); FAIL();}
        free(it.value);
    }
    if (counter != counts->size(counts)) {fprintf(stderr, "Iterator size does not match Hashtable size!\n"); FAIL();}

    counts->freeThis(counts); // Check with `leaks` on mac, or `valgrind` on linux!
}

TEST(VCellTest, CustomCLinkedListTest) {
LinkedList* list = create_String_LinkedList(false);
    std::string hello{"Hello"};
    std::string world{"World"};
    std::string fair{"Fair"};
    std::string smol{"Smoldyn"};
    std::string exclaim{"!"};
    std::string huh{"HUH"};

    std::string vecAsStr;

    list->append(list, (void*)hello.c_str());
    if (std::string{"Hello->0"} != (vecAsStr = getLinkedListAsSingleString(list))) {fprintf(stderr, "First element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}

    list->append(list, (void*)world.c_str());
    if (std::string{"Hello->World->0"} != (vecAsStr = getLinkedListAsSingleString(list))) {fprintf(stderr, "Second element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}

    list->append(list, (void*)exclaim.c_str());
    if (std::string{"Hello->World->!->0"} != (vecAsStr = getLinkedListAsSingleString(list))) {fprintf(stderr, "Third element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}

    char* old_value = (char*)list->replace(list, 1, (void*)fair.c_str());
    if (std::string{"Hello->Fair->!->0"} != (vecAsStr = getLinkedListAsSingleString(list))) {fprintf(stderr, "Replacement of second element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}
    if (std::string{"World"} != std::string{old_value}) {fprintf(stderr, "Old value not correct! (%s)\n", vecAsStr.c_str()); FAIL();}

    list->insert(list, 2, (void*)smol.c_str());
    if (std::string{"Hello->Fair->Smoldyn->!->0"} != (vecAsStr = getLinkedListAsSingleString(list))) {fprintf(stderr, "Insertion of element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}

    list->insert(list, list->_size, (void*)huh.c_str());
    if (std::string{"Hello->Fair->Smoldyn->!->HUH->0"} != (vecAsStr = getLinkedListAsSingleString(list))) {fprintf(stderr, "Insert-appended element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}

    list->remove(list, list->_size - 1);
    if (std::string{"Hello->Fair->Smoldyn->!->0"} != (vecAsStr = getLinkedListAsSingleString(list))) {fprintf(stderr, "Removal of element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}

    list->clear(list);
    if (std::string{"0"} != getLinkedListAsSingleString(list)) {fprintf(stderr, "Empty list not empty!\n"); FAIL();}

    list->freeThis(list); // Check with `leaks` on mac, or `valgrind` on linux!
}

TEST(VCellTest, CustomCVectorTest) {
    Vector* list = create_String_Vector(2, false);
    std::string hello{"Hello"};
    std::string world{"World"};
    std::string fair{"Fair"};
    std::string smol{"Smoldyn"};
    std::string exclaim{"!"};
    std::string huh{"HUH"};

    std::string vecAsStr;

    list->append(list, (void*)hello.c_str());
    if (std::string{"[ Hello ]"} != (vecAsStr = getVectorAsSingleString(list))) {fprintf(stderr, "First element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}
    if (2 != list->_allocated_size) {fprintf(stderr, "Allocated size #1 unexpected (`%d` is not `%d`)\\n", list->_allocated_size, 2); FAIL();}

    list->append(list, (void*)world.c_str());
    if (std::string{"[ Hello World ]"} != (vecAsStr = getVectorAsSingleString(list))) {fprintf(stderr, "Second element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}
    if (4 != list->_allocated_size) {fprintf(stderr, "Allocated size #2 unexpected (`%d` is not `%d`)\\n", list->_allocated_size, 4); FAIL();}

    list->append(list, (void*)exclaim.c_str());
    if (std::string{"[ Hello World ! ]"} != (vecAsStr = getVectorAsSingleString(list))) {fprintf(stderr, "Third element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}
    if (8 != list->_allocated_size) {fprintf(stderr, "Allocated size #3 unexpected (`%d` is not `%d`)\\n", list->_allocated_size, 8); FAIL();}

    char* old_value = (char*)list->replace(list, 1, (void*)fair.c_str());
    if (std::string{"[ Hello Fair ! ]"} != (vecAsStr = getVectorAsSingleString(list))) {fprintf(stderr, "Replacement of second element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}
    if (std::string{"World"} != std::string{old_value}) {fprintf(stderr, "Old value not correct! (%s)\n", vecAsStr.c_str()); FAIL();}
    if (8 != list->_allocated_size) {fprintf(stderr, "Allocated size #4 unexpected (`%d` is not `%d`)\\n", list->_allocated_size, 8); FAIL();}

    list->insert(list, 2, (void*)smol.c_str());
    if (std::string{"[ Hello Fair Smoldyn ! ]"} != (vecAsStr = getVectorAsSingleString(list))) {fprintf(stderr, "Insertion of element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}
    if (8 != list->_allocated_size) {fprintf(stderr, "Allocated size #5 unexpected (`%d` is not `%d`)\\n", list->_allocated_size, 8); FAIL();}

    list->insert(list, list->_utilized_size, (void*)huh.c_str());
    if (std::string{"[ Hello Fair Smoldyn ! HUH ]"} != (vecAsStr = getVectorAsSingleString(list))) {fprintf(stderr, "Insert-appended element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}
    if (8 != list->_allocated_size) {fprintf(stderr, "Allocated size #6 unexpected (`%d` is not `%d`)\\n", list->_allocated_size, 8); FAIL();}

    list->remove(list, list->_utilized_size - 1);
    if (std::string{"[ Hello Fair Smoldyn ! ]"} != (vecAsStr = getVectorAsSingleString(list))) {fprintf(stderr, "Removal of element not correct! (%s)\n", vecAsStr.c_str()); FAIL();}
    if (8 != list->_allocated_size) {fprintf(stderr, "Allocated size #7 unexpected (`%d` is not `%d`)\\n", list->_allocated_size, 8); FAIL();}

    list->clear(list);
    if (std::string{"[ ]"} != getVectorAsSingleString(list)) {fprintf(stderr, "Empty list not empty!\n"); FAIL();}
    if (2 != list->_allocated_size) {fprintf(stderr, "Allocated size #8 unexpected (`%d` is not `%d`)\\n", list->_allocated_size, 8); FAIL();}

    list->freeThis(list); // Check with `leaks` on mac, or `valgrind` on linux!
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

static std::string getVectorAsSingleString(Vector* list){
    std::string vectorString{"[ "};
    for (int i = 0; i < list->_utilized_size; i++) {
        vectorString += std::string{static_cast<char *>(list->get(list, i))} + " ";
    }
    return vectorString + "]";
}

static std::string getLinkedListAsSingleString(LinkedList* list){
    if (list->_size == 0) return "0";
    std::string vectorString{};
    LinkedList_Node* currentNode = list->_head;
    while (currentNode) {
        vectorString += std::string{static_cast<char *>(currentNode->data)} + "->";
        currentNode = currentNode->next;
    }
    return vectorString + "0";
}
