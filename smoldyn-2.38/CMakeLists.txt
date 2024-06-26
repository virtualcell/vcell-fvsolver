# Main CMakeLists.txt file to build Smoldyn, its libraries, and its utilities using CMake.
# Written by Steve Andrews, April 16, 2012.
# This file, and other files of the Smoldyn project, are licensed with LGPL license.
# The primary Smoldyn web site is http://www.smoldyn.org.
# Please report bugs and problems to support@smoldyn.org.

########## Basic setup ##########

project(smoldyn C CXX)
#cmake_minimum_required(VERSION 2.6)
set(SMOLDYN_VERSION "2.38")


######### Targets to build ###########

option(OPTION_TARGET_SMOLDYN_SOLVER "Create stand-alone Smoldyn program" ON)
option(OPTION_TARGET_LIBSMOLDYN "Create LibSmoldyn library" OFF)


####### Compiling options ##########
option(OPTION_VCELL "Compile Smoldyn for VCell" OFF)
option(OPTION_NSV "Compile Smoldyn with NextSubvolume functionality" ON)
option(OPTION_PDE "Compile Smoldyn with PDE functionality" OFF)
option(OPTION_VTK "Compile Smoldyn with VTK functionality" OFF)
option(OPTION_STATIC "Compile Smoldyn with static libraries" OFF)
option(OPTION_MINGW "Cross-compile for Windows using MinGW compiler" OFF)
option(OPTION_USE_OPENGL "Build with OpenGL support" ON)
option(OPTION_USE_ZLIB "Build with Zlib support" OFF)

option(OPTION_USE_LIBTIFF "Build with LibTiff support" ON)
option(OPTION_USE_ICONV "Build with Libiconv support" OFF)

if (OPTION_NSV)
	set(OPTION_LATTICE ON)
endif (OPTION_NSV)

if (OPTION_PDE)
	set(OPTION_LATTICE ON)
endif (OPTION_PDE)

if (OPTION_STATIC)
	set(OPTION_USE_ZLIB ON)
endif(OPTION_STATIC)


######### Core code information ###########

set(HEADER_FILES
	source/lib/Geometry.h
	source/lib/List.h
	source/lib/math2.h
	source/lib/opengl2.h
	source/lib/parse.h
	source/lib/queue.h
	source/lib/random2.h
	source/lib/Rn.h
	source/lib/RnSort.h
	source/lib/rxnparam.h
	source/lib/SimCommand.h
	source/lib/Sphere.h
	source/lib/string2.h
	source/lib/SurfaceParam.h
	source/lib/Zn.h
	source/lib/SFMT/SFMT.h
	source/Smoldyn/smoldyn.h
	source/Smoldyn/smoldynfuncs.h
	source/NextSubVolume/nsvc.h
	source/vtk/vtkwrapper.h
)

set(SRC_FILES
	source/lib/Geometry.c
	source/lib/List.c
	source/lib/math2.c
	source/lib/opengl2.c
	source/lib/parse.c
	source/lib/queue.c
	source/lib/random2.c
	source/lib/Rn.c
	source/lib/RnSort.c
	source/lib/rxnparam.c
	source/lib/SimCommand.c
	source/lib/Sphere.c
	source/lib/string2.c
	source/lib/SurfaceParam.c
	source/lib/Zn.c
	source/lib/SFMT/SFMT.c
	source/Smoldyn/smolbng.c
	source/Smoldyn/smolboxes.c
	source/Smoldyn/smolcmd.c
	source/Smoldyn/smolcomparts.c
	source/Smoldyn/smolfilament.c
	source/Smoldyn/smolgraphics.c
	source/Smoldyn/smolmolec.c
	source/Smoldyn/smolport.c
	source/Smoldyn/smollattice.c
	source/Smoldyn/smolreact.c
	source/Smoldyn/smolsim.c
	source/Smoldyn/smolsurface.c
	source/Smoldyn/smolwall.c
)

set(MAIN_FILES source/Smoldyn/smoldyn.c)
set(SMOLDYN_EXE smoldyn)

if(OPTION_TARGET_LIBSMOLDYN)
	list(APPEND HEADER_FILES source/Smoldyn/libsmoldyn.h)
	list(APPEND SRC_FILES source/Smoldyn/libsmoldyn.c)
endif(OPTION_TARGET_LIBSMOLDYN)

if(OPTION_VCELL)
	list(APPEND SRC_FILES
		source/Smoldyn/smoldynhybrid.c
		)
endif(OPTION_VCELL)

set_source_files_properties(${SRC_FILES} PROPERTIES LANGUAGE CXX )
set_source_files_properties(${MAIN_FILES} PROPERTIES LANGUAGE CXX )

include_directories(source/lib source/Smoldyn source/NextSubVolume source/vtk ${CMAKE_CURRENT_BINARY_DIR})

####### Build for debugging or release ##########

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(COMPILER_IS_CLANGXX TRUE)
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

if(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANGXX)
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wno-deprecated-declarations")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall  -Wno-deprecated-declarations -DNDEBUG")
endif(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANGXX)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the buid type: None, Debug, Release, RelWithDebInfo, or MinSizeRel" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


####### Option: Compile with MinGW ##########

if (OPTION_MINGW)
	message(STATUS "Cross-compiling for Windows using MinGW libraries from: ${MINGWDIR}")
endif(OPTION_MINGW)


######### Platform dependent information ##########
if (OPTION_VCELL)
	if (OPTION_USE_OPENGL)
		if (MSVC)
			add_definitions(-D_CRT_SECURE_NO_WARNINGS)
			include_directories(${CMAKE_CURRENT_SOURCE_DIR}/windows/glut-3.7.6)
#			link_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/vcell ${CMAKE_CURRENT_SOURCE_DIR}/windows/glut-3.7.6)
			if (ARCH_64bit)
				list(APPEND DEP_LIBS glut64)
			else()
				list(APPEND DEP_LIBS glut32 zlib)
			endif()
		endif (MSVC)
	endif (OPTION_USE_OPENGL)
elseif (OPTION_MINGW)
	include_directories(${MINGWDIR}/include windows)
	link_directories(${MINGWDIR}/lib)
	#list(APPEND DEP_LIBS opengl32 glu32 freeglut z)
#	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
#	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static -s")
# 	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static -s")
endif()

set(APPLE_BUILD 0)
set(NIX_BUILD 0)
set(WINDOWS_BUILD 0)
if(OPTION_MINGW)
	set(WINDOWS_BUILD 1)
elseif(APPLE)
	set(APPLE_BUILD 1)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.5")
else()
	set(NIX_BUILD 1)
endif()


####### Option: Build with OpenGL (gl and glu only, not glut) ##########

if (OPTION_USE_OPENGL AND OPTION_VCELL)
	set(HAVE_OPENGL TRUE)
	set(HAVE_GL_GLU_H TRUE)
	set(HAVE_GL_GL_H TRUE)
elseif(OPTION_USE_OPENGL)
	if(OPTION_MINGW)
		find_file(HAVE_GL_GL_H GL/gl.h PATHS ${MINGWDIR}/include)
		find_file(HAVE_OPENGL_GL_H OpenGL/gl.h PATHS ${MINGWDIR}/include)
		find_file(HAVE_GL_GLU_H GL/glu.h PATHS ${MINGWDIR}/include)
		find_file(HAVE_OPENGL_GLU_H OpenGL/glu.h PATHS ${MINGWDIR}/include)

		#find_path(OPEN_GL_INCLUDE_DIR gl.h PATHS ${MINGWDIR}/include/GL)
		get_filename_component(OPEN_GL_INCLUDE_DIR "${HAVE_GL_GL_H}" PATH)
		find_library(OPEN_GL_LIBRARIES libopengl32.a PATHS ${MINGWDIR}/lib)
		find_library(GLU32_LIBRARIES libglu32.a PATHS ${MINGWDIR}/lib)
	else()
		find_file(HAVE_GL_GL_H GL/gl.h)
		find_file(HAVE_OPENGL_GL_H OpenGL/gl.h)
		find_file(HAVE_GL_GLU_H GL/glu.h)
		find_file(HAVE_OPENGL_GLU_H OpenGL/glu.h)
		include(FindOpenGL)
	endif()

	if(OPENGL_FOUND)
		set(OPENGL_FOUND TRUE)
		message(STATUS "OpenGL found: '${OPENGL_INCLUDE_DIR}', '${OPENGL_LIBRARIES}'")
	elseif(OPEN_GL_INCLUDE_DIR AND OPEN_GL_LIBRARIES)
		set(OPENGL_FOUND TRUE)
		message(STATUS "OpenGL found: '${OPEN_GL_INCLUDE_DIR}', '${OPEN_GL_LIBRARIES}'")
	else()
		set(OPENGL_FOUND FALSE)
		message(FATAL_ERROR "OpenGL not found.  Building without OpenGL is possible.")
	endif()

	if(OPTION_MINGW)
		if(GLU32_LIBRARIES)
			message(STATUS "Glu32 found: '${GLU32_LIBRARIES}'")
		else()
			message(FATAL_ERROR "Glu32 not found.  Building without OpenGL is possible.")
		endif()
	endif(OPTION_MINGW)

	set(HAVE_OPENGL TRUE)
	include_directories(${OPENGL_INCLUDE_DIR} ${OPEN_GL_INCLUDE_DIR})
	list(APPEND DEP_LIBS ${OPENGL_LIBRARIES} ${OPEN_GL_LIBRARIES} ${GLU32_LIBRARIES})
endif()


####### Option: Build with OpenGL (glut only) ##########

if (OPTION_USE_OPENGL AND OPTION_VCELL)
	set(HAVE_GL_GLUT_H TRUE)
elseif(OPTION_USE_OPENGL)
	if(OPTION_MINGW)
		find_file(HAVE_GL_GLUT_H GL/glut.h PATHS ${MINGWDIR}/include)
		find_file(HAVE_GLUT_GLUT_H GLUT/glut.h PATHS ${MINGWDIR}/include)

		get_filename_component(GLUT_INCLUDE_DIR "${HAVE_GL_GLUT_H}" PATH)
		#find_path(GLUT_INCLUDE_DIR glut.h PATHS ${MINGWDIR}/include/GL)
		find_library(GLUT_LIBRARIES libfreeglut.a PATHS ${MINGWDIR}/lib)
#	elseif(OPTION_STATIC)
#		find_file(HAVE_GL_FREEGLUT_H GL/freeglut.h)
#		find_path(GLUT_INCLUDE_DIR GL/freeglut.h)
#		find_library(GLUT_LIBRARIES libglut.a)
	else()
		find_file(HAVE_GL_GLUT_H GL/glut.h)
		find_file(HAVE_GLUT_GLUT_H GLUT/glut.h)
		include(FindGLUT)
		#find_file(HAVE_GL_FREEGLUT_H GL/freeglut.h)
		#find_path(GLUT_INCLUDE_DIR GL/freeglut.h)
		#find_library(GLUT_LIBRARIES NAMES libglut)
	endif()

	if(GLUT_FOUND)
		set(GLUT_FOUND TRUE)
		message(STATUS "Glut found: '${GLUT_INCLUDE_DIR}', '${GLUT_LIBRARIES}'")
	elseif(GLUT_INCLUDE_DIR AND GLUT_LIBRARIES)
		set(GLUT_FOUND TRUE)
		message(STATUS "Glut found: '${GLUT_INCLUDE_DIR}', '${GLUT_LIBRARIES}'")
	else()
		set(GLUT_FOUND FALSE)
		message(FATAL_ERROR "Glut not found.  Building without OpenGL is possible.")
	endif()

	set(HAVE_OPENGL TRUE)
	include_directories(${GLUT_INCLUDE_DIR})
	list(APPEND DEP_LIBS ${GLUT_LIBRARIES})
endif(OPTION_USE_OPENGL AND OPTION_VCELL)


####### Build with LibX11 (only required for static build on Mac) ##########

#if(OPTION_STATIC AND OPTION_USE_OPENGL AND HAVE_OPENGL)
#	FIND_PACKAGE(X11)
#	if(X11_FOUND)
#		message(STATUS "Found X11: '${X11_INCLUDE_DIR}', '${X11_LIBRARIES}'")
#	else()
#		message(FATAL_ERROR "X11 not found")
#	endif()
#	include_directories(${X11_INCLUDE_DIR})
#	list(APPEND DEP_LIBS ${X11_LIBRARIES})
#endif()


####### Option: Build with LibTiff ##########

if(OPTION_USE_LIBTIFF)
	if(NOT HAVE_OPENGL)
		message(WARNING "LibTiff won't be used because build does not include OpenGL.")
	endif()

	if(OPTION_MINGW)
		find_path(TIFF_INCLUDE_DIR tiff.h PATHS ${MINGWDIR}/include NO_DEFAULT_PATH)
		find_library(TIFF_LIBRARY libtiff.a PATHS ${MINGWDIR}/lib NO_DEFAULT_PATH)
	elseif(OPTION_STATIC)
		find_path(TIFF_INCLUDE_DIR tiff.h)
		find_library(TIFF_LIBRARY libtiff.a)
	else()
		FIND_PACKAGE(TIFF)
	endif()

	if(TIFF_INCLUDE_DIR AND TIFF_LIBRARY)
		set(HAVE_LIBTIFF TRUE)
		message(STATUS "Found Libtiff: '${TIFF_INCLUDE_DIR}', '${TIFF_LIBRARY}'")
	else()
		set(HAVE_LIBTIFF FALSE)
		message(FATAL_ERROR "Libtiff not found")
	endif()

	include_directories(${TIFF_INCLUDE_DIR})
	list(APPEND DEP_LIBS ${TIFF_LIBRARY})
endif(OPTION_USE_LIBTIFF)


####### Option: Build with Zlib ##########

if(OPTION_USE_ZLIB)
	if(OPTION_MINGW)
		find_path(ZLIB_INCLUDE_DIRS zlib.h PATHS ${MINGWDIR}/include NO_DEFAULT_PATH)
		find_library(ZLIB_LIBRARIES libz.a PATHS ${MINGWDIR}/lib NO_DEFAULT_PATH)
	elseif(OPTION_STATIC)
		find_path(ZLIB_INCLUDE_DIRS zlib.h)
		find_library(ZLIB_LIBRARIES libz.a)
	else()
		include(FindZLIB)
	endif()

	if(ZLIB_INCLUDE_DIRS AND ZLIB_LIBRARIES)
		set(HAVE_ZLIB TRUE)
		message(STATUS "Found Zlib: '${ZLIB_INCLUDE_DIRS}', '${ZLIB_LIBRARIES}'")
	else()
		set(HAVE_ZLIB FALSE)
		message(FATAL_ERROR "Zlib not found")
	endif()

	include_directories(${ZLIB_INCLUDE_DIRS})
	list(APPEND DEP_LIBS ${ZLIB_LIBRARIES})
endif(OPTION_USE_ZLIB)


####### Option: Build with iconv ##########

if(OPTION_USE_ICONV)
	if(OPTION_MINGW)
		find_path(ICONV_INCLUDE_DIRS iconv.h PATHS ${MINGWDIR}/include NO_DEFAULT_PATH)
		find_library(ICONV_LIBRARIES libiconv.a PATHS ${MINGWDIR}/lib NO_DEFAULT_PATH)
	elseif(OPTION_STATIC)
		find_path(ICONV_INCLUDE_DIRS iconv.h)
		find_library(ICONV_LIBRARIES libiconv.a)
	else()
		find_path(ICONV_INCLUDE_DIRS iconv.h)
		find_library(ICONV_LIBRARIES NAMES iconv)
	endif()

	if(ICONV_INCLUDE_DIRS AND ICONV_LIBRARIES)
		set(HAVE_ICONV TRUE)
		message(STATUS "Found Libiconv: '${ICONV_INCLUDE_DIRS}', '${ICONV_LIBRARIES}'")
	else()
		set(HAVE_ICONV FALSE)
		message(FATAL_ERROR "Libiconv not found")
	endif()

	include_directories(${ICONV_INCLUDE_DIRS})
	list(APPEND DEP_LIBS ${ICONV_LIBRARIES})
endif(OPTION_USE_ICONV)


####### Option: Build with VTK ##########

if(OPTION_VTK)
	add_definitions(-DHAVE_VTK)
	find_package(VTK REQUIRED)
	if(VTK_FOUND)
		message(STATUS "Found VTK: '${VTK_INCLUDE_DIRS}'")
	else()
		message(FATAL_ERROR "VTK not found")
	endif()

	include_directories(${VTK_INCLUDE_DIRS})

	list(APPEND SRC_FILES source/vtk/vtkwrapper.cpp)
	list(APPEND HEADER_FILES source/vtk/vtkwrapper.h)
	add_definitions("-Wno-deprecated")
	list(APPEND DEP_LIBS ${VTK_LIBRARIES})

endif(OPTION_VTK)

####### Option: Build with NextSubvolume ##########

if(OPTION_NSV)
	#add_subdirectory(source/NextSubVolume)
	#list(APPEND DEP_LIBS nsv)
	#include_directories(source/NextSubVolume)
	#include_directories(${VTK_INCLUDE_DIRS})
	
	if(OPTION_MINGW)
		find_path(SIGNAL_H_DIR signal.h PATHS ${MINGWDIR}/include NO_DEFAULT_PATH)
		if(SIGNAL_H_DIR)
			message(STATUS "Found signal.h: '${SIGNAL_H_DIR}'")
		else()
			message(FATAL_ERROR "signal.h not found")
		endif()
		include_directories(${SIGNAL_H_DIR})
	endif()

	list(APPEND SRC_FILES 
					source/NextSubVolume/StructuredGrid.cpp
					source/NextSubVolume/Species.cpp
					source/NextSubVolume/ReactionEquation.cpp
					source/NextSubVolume/NextSubvolumeMethod.cpp
					source/NextSubVolume/Kairos.cpp
					source/NextSubVolume/Io.cpp
					source/NextSubVolume/Geometry.cpp
					source/NextSubVolume/nsvc.cpp
					)
	list(APPEND HEADER_FILES 
					source/NextSubVolume/Vector.h
					source/NextSubVolume/StructuredGrid.h
					source/NextSubVolume/Species.h
					source/NextSubVolume/ReactionEquation.h
					source/NextSubVolume/NextSubvolumeMethod.h
					source/NextSubVolume/Log.h
					source/NextSubVolume/Kairos.h
					source/NextSubVolume/Io.h
					source/NextSubVolume/Geometry.h
					source/NextSubVolume/Constants.h
					source/NextSubVolume/nsvc.h
					)
						
	add_definitions("-Wno-deprecated")
endif(OPTION_NSV)

####### Option: Build with PDE ##########

####### Targets ##########

#if(OPTION_TARGET_SMOLDYN_SOLVER)
#	add_executable(${SMOLDYN_EXE} ${SRC_FILES} ${MAIN_FILES} ${HEADER_FILES})
#	target_link_libraries(${SMOLDYN_EXE} ${DEP_LIBS})
#endif()

#if(OPTION_TARGET_LIBSMOLDYN)
#	if (NOT OPTION_VCELL)
#		add_library(smoldyn_shared SHARED ${SRC_FILES} ${HEADER_FILES})
#		target_link_libraries(smoldyn_shared ${DEP_LIBS} vcellmessaging)
#		target_include_directories(smoldyn_shared PUBLIC 
#			$<BUILD_INTERFACE:/${CMAKE_CURRENT_SOURCE_DIR}/source/lib>
#			$<BUILD_INTERFACE:/${CMAKE_CURRENT_SOURCE_DIR}/source/Smoldyn>
#			$<BUILD_INTERFACE:/${CMAKE_CURRENT_SOURCE_DIR}/source/NextSubvolume>
#			$<BUILD_INTERFACE:/${PROJECT_BINARY_DIR}/>
#			$<INSTALL_INTERFACE:/.>
#		)
#	endif (NOT OPTION_VCELL)
	add_library(smoldyn_static STATIC ${SRC_FILES} ${HEADER_FILES})
	target_link_libraries(smoldyn_static vcellsmoldynbridge vcellmessaging)
	target_include_directories(smoldyn_static PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/source/lib
		${CMAKE_CURRENT_SOURCE_DIR}/source/Smoldyn
		${CMAKE_CURRENT_SOURCE_DIR}/source/NextSubvolume
		${CMAKE_CURRENT_BINARY_DIR}
	)
	if(APPLE)
		#SET_TARGET_PROPERTIES(smoldyn_shared PROPERTIES LINK_FLAGS "-framework Glut -framework OpenGL")
	endif(APPLE)
#endif(OPTION_TARGET_LIBSMOLDYN)


########## install ###########

if(NOT OPTION_MINGW) 

#	if(OPTION_TARGET_SMOLDYN_SOLVER)
#		if(NOT OPTION_VCELL)
#			install(TARGETS ${SMOLDYN_EXE} RUNTIME DESTINATION bin)
#		else(NOT OPTION_VCELL)
#			install(TARGETS ${SMOLDYN_EXE} RUNTIME DESTINATION
#				${OPTION_EXE_DIRECTORY})
#		endif(NOT OPTION_VCELL)
#	endif()

	if(OPTION_TARGET_LIBSMOLDYN)
		if(NOT OPTION_VCELL)
			install(TARGETS smoldyn_shared LIBRARY DESTINATION lib)
		endif(NOT OPTION_VCELL)
		install(TARGETS smoldyn_static ARCHIVE DESTINATION lib)
		install(FILES source/Smoldyn/libsmoldyn.h source/Smoldyn/smoldyn.h ${CMAKE_CURRENT_BINARY_DIR}/smoldynconfigure.h DESTINATION include)
	endif(OPTION_TARGET_LIBSMOLDYN)

endif(NOT OPTION_MINGW) 


########## configure file #########

configure_file (
	${CMAKE_CURRENT_SOURCE_DIR}/source/smoldynconfigure.h.in
	${CMAKE_CURRENT_BINARY_DIR}/smoldynconfigure.h
)

########## package #########

include(CPack)



