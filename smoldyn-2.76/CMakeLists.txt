# VCell build of the vendored Smoldyn 2.76 (see UPSTREAM.md).
# This replaces upstream's CMakeLists.txt: it builds only the smoldyn_static library that
# bridgeVCellSmoldyn and the FV solver link against, always with OPTION_VCELL.
# Smoldyn is licensed under the LGPL; see LICENSE.

project(smoldyn C CXX)
set(SMOLDYN_VERSION "2.76")

if (NOT OPTION_VCELL)
	message(FATAL_ERROR "smoldyn-2.76 is only set up to build for VCell (OPTION_VCELL=ON)")
endif()
if (OPTION_NSV)
	message(FATAL_ERROR "smoldyn-2.76 does not include NextSubVolume; build with OPTION_NSV=OFF")
endif()
if (OPTION_USE_OPENGL)
	message(FATAL_ERROR "smoldyn-2.76 is not set up for OpenGL; build with OPTION_USE_OPENGL=OFF")
endif()


######### Core code information ###########

set(HEADER_FILES
	source/libSteve/Geometry.h
	source/libSteve/List.h
	source/libSteve/math2.h
	source/libSteve/opengl2.h
	source/libSteve/parse.h
	source/libSteve/queue.h
	source/libSteve/random2.h
	source/libSteve/Rn.h
	source/libSteve/RnSort.h
	source/libSteve/RnSparse.h
	source/libSteve/rxnparam.h
	source/libSteve/SimCommand.h
	source/libSteve/Sphere.h
	source/libSteve/string2.h
	source/libSteve/SurfaceParam.h
	source/libSteve/Zn.h
	source/libSteve/SFMT/SFMT.h
	source/libSteve/ComplexDataStructures/LinkedList.h
	source/libSteve/ComplexDataStructures/LinkedListNode.h
	source/libSteve/ComplexDataStructures/Hashtable.h
	source/libSteve/ComplexDataStructures/Vector.h
	source/Smoldyn/smoldyn.h
	source/Smoldyn/smoldynfuncs.h
	source/Smoldyn/libsmoldyn.h
)

set(SRC_FILES
	source/libSteve/Geometry.c
	source/libSteve/List.c
	source/libSteve/math2.c
	source/libSteve/opengl2.c
	source/libSteve/parse.c
	source/libSteve/queue.c
	source/libSteve/random2.c
	source/libSteve/Rn.c
	source/libSteve/RnSort.c
	source/libSteve/RnSparse.c
	source/libSteve/rxnparam.c
	source/libSteve/SimCommand.c
	source/libSteve/Sphere.c
	source/libSteve/string2.c
	source/libSteve/SurfaceParam.c
	source/libSteve/Zn.c
	source/libSteve/SFMT/SFMT.c
	source/libSteve/ComplexDataStructures/LinkedList.c
	source/libSteve/ComplexDataStructures/LinkedListNode.c
	source/libSteve/ComplexDataStructures/Hashtable.c
	source/libSteve/ComplexDataStructures/Vector.c
	source/Smoldyn/smolbng.c
	source/Smoldyn/smolboxes.c
	source/Smoldyn/smolcmd.c
	source/Smoldyn/smolcomparts.c
	source/Smoldyn/smolfilament.c
	source/Smoldyn/smolhelp.c
	source/Smoldyn/smolgraphics.c
	source/Smoldyn/smolmolec.c
	source/Smoldyn/smolport.c
	source/Smoldyn/smollattice.c
	source/Smoldyn/smolreact.c
	source/Smoldyn/smolrule.c
	source/Smoldyn/smolsim.cpp
	source/Smoldyn/smolsurface.c
	source/Smoldyn/smolwall.c
	source/Smoldyn/smoldynhybrid.c
	source/Smoldyn/vcell_smoldyn.cpp
)

if (OPTION_TARGET_LIBSMOLDYN)
	list(APPEND SRC_FILES source/Smoldyn/libsmoldyn.cpp)
endif()

# The VCell declarations in smoldyn.h are C++, so every translation unit is compiled as C++
# (upstream compiles libSteve as C, which fails with OPTION_VCELL).
set_source_files_properties(${SRC_FILES} PROPERTIES LANGUAGE CXX)


######### Configuration header ###########

if (APPLE)
	set(APPLE_BUILD ON)
elseif (WIN32)
	set(WINDOWS_BUILD ON)
else()
	set(NIX_BUILD ON)
endif()
set(HAVE_ZLIB TRUE)
# smolbng.c needs a default BioNetGen path; VCell does not use BioNetGen.
set(BNG2_PATH "BNG2.pl")

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/source/smoldynconfigure.h.in
	${CMAKE_CURRENT_BINARY_DIR}/smoldynconfigure.h
)


######### Target ###########

add_library(smoldyn_static STATIC ${SRC_FILES} ${HEADER_FILES})
target_link_libraries(smoldyn_static vcellsmoldynbridge vcellmessaging)
target_compile_definitions(smoldyn_static PRIVATE -D_CRT_SECURE_NO_WARNINGS)
# Lets bridgeVCellSmoldyn adapt to API differences between the vendored Smoldyn versions.
target_compile_definitions(smoldyn_static PUBLIC VCELL_SMOLDYN_VERSION=276)
# Every source here is compiled as C++, so libSteve's functions have C++ linkage; this turns
# off the extern "C" blocks that some libSteve headers declare them in.
target_compile_definitions(smoldyn_static PUBLIC SMOLDYN_CXX_LINKAGE)
target_include_directories(smoldyn_static PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/source/libSteve
	${CMAKE_CURRENT_SOURCE_DIR}/source/Smoldyn
	${CMAKE_CURRENT_BINARY_DIR}
)

if (OPTION_TARGET_LIBSMOLDYN)
	install(TARGETS smoldyn_static ARCHIVE DESTINATION lib)
	install(FILES source/Smoldyn/libsmoldyn.h source/Smoldyn/smoldyn.h ${CMAKE_CURRENT_BINARY_DIR}/smoldynconfigure.h DESTINATION include)
endif()
