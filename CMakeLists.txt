cmake_minimum_required(VERSION 3.0)

project(PXTNPLAY VERSION 0.0.2 LANGUAGES CXX)


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# required package
#     pkgconfig + libao + vorbis(vorbisfile)
#
find_package(PkgConfig REQUIRED)

pkg_search_module(AO REQIRED ao)
pkg_search_module(VORBISFILE REQIRED vorbisfile)

#
# get pxtone library version string
# (ex static s32 _version = 20060111)
#
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/src/pxtone/pxtnWoicePTV.cpp PXTONE_VERSION_LINE REGEX
	"^static.*_version.*=.*[0-9]+.*$")
string(REGEX MATCH [[_version.*=[^0-9]*([0-9]+)]] PXTONE_VERSION_MATCH ${PXTONE_VERSION_LINE})
if(${CMAKE_MATCH_1})
	set(PXTONE_VERSION ${CMAKE_MATCH_1})
else(${CMAKE_MATCH_1})
	message( FATAL_ERROR "You can not get PXTONE_VERSION, CMake will exit." )
endif(${CMAKE_MATCH_1})


file(GLOB PXTNPLAY_SRC
	"./src/pxtone/*.h"
	"./src/pxtone/*.cpp"
	"./src/*.cpp"
	"./src/*.h"
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
	${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

add_compile_options(-Wall -Wextra -Woverloaded-virtual -Wtype-limits)
add_compile_options(-DpxINCLUDE_OGGVORBIS)
add_compile_options(${AO_CFLAGS})
add_compile_options(${VORBISFILE_CFLAGS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/pxtone)
include_directories(${AO_INCLUDE_DIRS})
include_directories(${VORBISFILE_INCLUDE_DIRS})

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

link_libraries(${AO_LDFLAGS})
link_libraries(${VORBISFILE_LDFLAGS})

if(APPLE)
	link_libraries("-liconv")
else(APPLE)
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
endif(APPLE)

add_executable(pxtnplay ${PXTNPLAY_SRC})

