project(PCGPack Fortran)    

if(NOT DEFINED CMAKE_Fortran_COMPILER)
	set(CMAKE_CXX_COMPILER "flang")
endif()

if (MSVC)
    set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /names:lowercase")
elseif (UNIX)
	set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static")
endif()

if (APPLE)
	#set (CMAKE_Fortran_COMPILER gfortran)
	set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -m64")
endif()
    
file (GLOB SRC_FILES *.f)   
add_library(PCGPack STATIC ${SRC_FILES})
install(TARGETS PCGPack 
	ARCHIVE DESTINATION bin)
