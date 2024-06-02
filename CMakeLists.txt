cmake_minimum_required(VERSION 3.22)
option(OPTION_TARGET_PYTHON_BINDING "build Python bindings" ON)
option(OPTION_TARGET_MESSAGING "Messaging (requires libcurl)" off)
option(OPTION_TARGET_DOCS "Generate Doxygen documentation" off)
option(OPTION_TARGET_FV_SOLVER on)
option(OPTION_VCELL "Compile Smoldyn for VCell" ON)
option(OPTION_NSV "Compile Smoldyn with NextSubvolume functionality" OFF)
option(OPTION_USE_OPENGL "Build with OpenGL support" OFF)
option(OPTION_USE_ZLIB "Build with Zlib support" ON)
option(OPTION_USE_LIBTIFF "Build with LibTiff support" OFF)
option(OPTION_USE_ICONV "Build with Libiconv support" OFF)


if (OPTION_TARGET_PYTHON_BINDING)
	project(
			${SKBUILD_PROJECT_NAME}
			VERSION ${SKBUILD_PROJECT_VERSION}
			LANGUAGES CXX C Fortran)
	set(OPTION_TARGET_MESSAGING OFF)
	set(OPTION_TARGET_FV_SOLVER ON)
	set(OPTION_TARGET_DOCS OFF)
	set(OPTION_VCELL ON)
	set(OPTION_NSV OFF)
	set(OPTION_USE_OPENGL OFF)
	set(OPTION_USE_ZLIB ON)
	set(OPTION_USE_LIBTIFF OFF)
	set(OPTION_USE_ICONV OFF)
else()
	project(
			fvsolver
			LANGUAGES CXX C Fortran)
	set(OPTION_TARGET_LIBSMOLDYN ON)
	set(OPTION_VCELL ON)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OPTION_USE_OPENGL OFF CACHE BOOL "")

if(POLICY CMP0057)
	cmake_policy(SET CMP0057 NEW)
endif()

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

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

include(GetGitRevisionDescription)
git_describe(GIT_DESCRIBE ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)

include (FindZLIB)

if (NOT OPTION_TARGET_PYTHON_BINDING)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

	if(DEFINED CMAKE_BUILD_TYPE)
	   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
	else()
	   SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
	endif()

	if (${OPTION_TARGET_DOCS})
		add_custom_target(graphviz ALL
					  "${CMAKE_COMMAND}" "--graphviz=dependencies.dot" .
					  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
	endif()
endif(NOT OPTION_TARGET_PYTHON_BINDING)

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

#############################################
# installation directories
#############################################
if (NOT OPTION_TARGET_PYTHON_BINDING)
	if (NOT OPTION_TARGET_MESSAGING)
		set(OPTION_EXE_DIRECTORY "bin" CACHE PATH "installation directory")
	else (NOT OPTION_TARGET_MESSAGING)
		set(OPTION_EXE_DIRECTORY "../bin" CACHE PATH "installation directory")
	endif (NOT OPTION_TARGET_MESSAGING)
	set(OPTION_LIB_DIRECTORY "lib" CACHE PATH "library directory")
	set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
endif (NOT OPTION_TARGET_PYTHON_BINDING)

add_definitions(-DUNIX)

if (OPTION_TARGET_MESSAGING)
	find_package(CURL REQUIRED)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	add_definitions(-DUSE_MESSAGING)
endif()

set(HDF5_USE_STATIC_LIBRARIES ON)
if(WINDOWS OR WIN32 OR WIN64)
	set(HDF5_USE_STATIC_LIBRARIES OFF)
	add_definitions(-DZIP_STATIC)
endif ()
#CMakeList.txt that comes with hdf5 switches library name if compiled with debug
find_package(HDF5 COMPONENTS HL CXX C)
set(HDF_VERSION ${HDF5_VERSION})
######    ${HDF5_HL_LIBRARIES} ${HDF5_CXX_LIBRARIES}

add_subdirectory(VCellMessaging)

add_subdirectory(VCellZipUtils)

add_subdirectory(libzip-1.2.0)

add_subdirectory(ExpressionParser)

add_subdirectory(sundials)

if (NOT APPLE)
	add_subdirectory(blas)
endif()

SET(HAVE_ZLIB TRUE)
add_subdirectory(bridgeVCellSmoldyn)
add_subdirectory(smoldyn-2.38)

add_subdirectory(VCell)
add_subdirectory(PCGPack)
add_subdirectory(qhull)

if (OPTION_TARGET_PYTHON_BINDING)
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

else(OPTION_TARGET_PYTHON_BINDING)

	include(FetchContent)
	FetchContent_Declare(
			googletest
			URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
	)
	if (WINDOWS)
		# For Windows: Prevent overriding the parent project's compiler/linker settings
		set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	endif()

	FetchContent_MakeAvailable(googletest)
	enable_testing()

	get_cmake_property(_variableNames VARIABLES)
	list (SORT _variableNames)
	foreach (_variableName ${_variableNames})
		message(STATUS "${_variableName}=${${_variableName}}")
	endforeach()

	include(CMakePrintHelpers)
	cmake_print_variables(OPTION_TARGET_MESSAGING
			OPTION_TARGET_DOCS
			OPTION_TARGET_FV_SOLVER
			OPTION_TARGET_LIBSMOLDYN OPTION_TARGET_VCELL OPTION_TARGET_NSV )
	cmake_print_variables(CMAKE_CXX_FLAGS CMAKE_C_FLAGS CMAKE_Fortran_FLAGS)
	cmake_print_variables(CMAKE_SYSTEM_NAME WINDOWS WIN32 MINGW APPLE ARCH_64bit ARCH_32bit)
	cmake_print_variables(CMAKE_CPP_COMPILER CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_Fortran_COMPILER)
	cmake_print_variables(HAVE_ZLIB)

endif (OPTION_TARGET_PYTHON_BINDING)