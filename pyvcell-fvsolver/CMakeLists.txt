cmake_minimum_required(VERSION 3.29)

project(
        ${SKBUILD_PROJECT_NAME}
        VERSION ${SKBUILD_PROJECT_VERSION}
        LANGUAGES CXX C Fortran)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0057)
    cmake_policy(SET CMP0057 NEW)
endif()

#############################################
#
#  Build 64bit binaries on Mac and target Macos 10.7 or later
#
##############################################
if (APPLE)
    execute_process(COMMAND uname -m OUTPUT_VARIABLE PROCESSOR_ARCHITECTURE OUTPUT_STRIP_TRAILING_WHITESPACE)
    message("Processor Architecture: ${PROCESSOR_ARCHITECTURE}")
    if (PROCESSOR_ARCHITECTURE STREQUAL "arm64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__arm64__")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__arm64__")
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__x86_64__")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__x86_64__")
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
    endif()
    #	set(CMAKE_Fortran_OSX_DEPLOYMENT_TARGET_FLAG "-mmacosx-version-min=10.7" CACHE PATH "")
    #	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE PATH "")
    #############################################
    #
    #  Choose 32bit or 64bit target arch on Linux
    #
    ##############################################
elseif (LINUX)
    option(LINUX_32bit_BINARIES "Build 32bit Linux BINARIES" OFF)
    option(LINUX_64bit_BINARIES "Build 64bit Linux BINARIES" ON)

    if (LINUX_32bit_BINARIES AND LINUX_64bit_BINARIES)
        message(FATAL_ERROR "It is required to select either 32bit or 64bit Linux Binaries, not both")
    endif()

    if (LINUX_32bit_BINARIES)
        set (ARCH_64bit FALSE)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
        set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -m32")
    endif()

    if (LINUX_64bit_BINARIES)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
        set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -m64")
    endif()
endif()

if(POLICY CMP0057)
    cmake_policy(SET CMP0057 NEW)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

include(GetGitRevisionDescription)
git_describe(GIT_DESCRIBE ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)

include (FindZLIB)

option(OPTION_TARGET_MESSAGING "Messaging (requires libcurl)" off)
option(OPTION_TARGET_DOCS "Generate Doxygen documentation" off)
option(OPTION_TARGET_FV_SOLVER on)
option(OPTION_TARGET_MESSAGING "Messaging (requires libcurl)" off)


if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "Choose minimum deploy target for Macos machines")
endif()

set(LINUX FALSE)
if (${CMAKE_SYSTEM_NAME} MATCHES Linux)
    set(LINUX TRUE)
endif()

set(WINDOWS FALSE)
if (${CMAKE_SYSTEM_NAME} MATCHES Windows)
    set(WINDOWS TRUE)
    set(WIN32 TRUE)
    set(MINGW TRUE)
endif()

set (ARCH_64bit FALSE)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (ARCH_64bit TRUE)
endif()

if (NOT APPLE AND NOT LINUX AND NOT MINGW)
    message(FATAL_ERROR "Unsupported Operating System or C/C++ Compiler.")
endif()

add_definitions(-DFORTRAN_UNDERSCORE)

set(OPTION_EXE_DIRECTORY "bin" CACHE PATH "installation directory")
set(OPTION_LIB_DIRECTORY "lib" CACHE PATH "library directory")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
add_definitions(-DUNIX)

set(HDF5_USE_STATIC_LIBRARIES ON)
if(WINDOWS OR WIN32 OR WIN64)
    set(HDF5_USE_STATIC_LIBRARIES OFF)
    add_definitions(-DZIP_STATIC)
endif ()
#CMakeList.txt that comes with hdf5 switches library name if compiled with debug
find_package(HDF5 COMPONENTS HL CXX C)
set(HDF_VERSION ${HDF5_VERSION})
######    ${HDF5_HL_LIBRARIES} ${HDF5_CXX_LIBRARIES}

add_subdirectory(../VCellMessaging bin/VCellMessaging)

add_subdirectory(../VCellZipUtils bin/VCellZipUtils)

add_subdirectory(../libzip-1.2.0 bin/libzip-1.2.0)

add_subdirectory(../ExpressionParser bin/ExpressionParser)

add_subdirectory(../sundials bin/sundials)

if (NOT APPLE)
    add_subdirectory(../blas bin/blas)
endif()

option(OPTION_VCELL "Compile Smoldyn for VCell" ON)
option(OPTION_NSV "Compile Smoldyn with NextSubvolume functionality" OFF)
option(OPTION_USE_OPENGL "Build with OpenGL support" OFF)
option(OPTION_USE_ZLIB "Build with Zlib support" ON)
option(OPTION_USE_LIBTIFF "Build with LibTiff support" OFF)
option(OPTION_USE_ICONV "Build with Libiconv support" OFF)
SET(HAVE_ZLIB TRUE)
set(OPTION_TARGET_LIBSMOLDYN ON)
set(OPTION_VCELL ON)
add_subdirectory(../bridgeVCellSmoldyn bin/bridgeVCellSmoldyn)
add_subdirectory(../smoldyn-2.38 bin/smoldyn-2.38)

add_subdirectory(../VCell bin/VCell)
add_subdirectory(../PCGPack bin/PCGPack)
add_subdirectory(../qhull bin/qhull)



# Find the module development requirements (requires FindPython from 3.17 or
# scikit-build-core's built-in backport)
find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

# Add a library using FindPython's tooling (pybind11 also provides a helper like this)
python_add_library(_core MODULE src/main.cpp src/SolverMain.cpp src/SolverMain.h WITH_SOABI)
target_link_libraries(_core PRIVATE vcell pybind11::headers)

# This is passing in the version as a define just as an example
target_compile_definitions(_core PRIVATE VERSION_INFO=${PROJECT_VERSION})

# The install directory is the output (wheel) directory
install(TARGETS _core DESTINATION pyvcell_fvsolver)