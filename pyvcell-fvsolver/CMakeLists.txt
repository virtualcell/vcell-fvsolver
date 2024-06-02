project(pyvcell_fvsolver)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADER_FILES
        include/SolverMain.h
)

set(SOURCE_FILES
        src/pyvcell_fvsolver.cpp
        src/SolverMain.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

pybind11_add_module(pyvcell_fvsolver ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(pyvcell_fvsolver PRIVATE vcell)
