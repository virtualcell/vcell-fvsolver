project(example)

set(HEADER_FILES
    include/add.h
)

set(SOURCE_FILES
    src/add.cpp
    src/example.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

pybind11_add_module(example ${SOURCE_FILES} ${HEADER_FILES})
