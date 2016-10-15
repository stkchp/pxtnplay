cmake_minimum_required(VERSION 3.0)

project(PXTNPLAY VERSION 0.0.1 LANGUAGES CXX)

find_package(ALSA REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(VORBISFILE REQIRED vorbisfile)

#
# get pxtone library version string
# (ex static s32 _version = 20060111)
#
file(STRINGS src/core/pxtnWoicePTV.cpp PXTONE_VERSION_LINE REGEX
	"^static.*_version.*=.*[0-9]+.*$")
string(REGEX MATCH [[_version.*=[^0-9]*([0-9]+)]] PXTONE_VERSION_MATCH ${PXTONE_VERSION_LINE})
if(${CMAKE_MATCH_1})
	set(PXTONE_VERSION ${CMAKE_MATCH_1})
else(${CMAKE_MATCH_1})
	message( FATAL_ERROR "You can not get PXTONE_VERSION, CMake will exit." )
endif(${CMAKE_MATCH_1})

file(GLOB PXTNPLAY_SRC
	"./src/core/*.h"
	"./src/core/*.cpp"
	"./src/play/*.h"
	"./src/play/*.cpp"
	"./src/pxwr/*.h"
	"./src/pxwr/*.cpp"
	"./src/*.cpp"
	"./src/*.h"
)

configure_file(src/config.h.in src/config.h)

add_compile_options(${ALSA_CFLAGS})
add_compile_options(${VORBISFILE_CFLAGS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/play)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/pxwr)
include_directories(${ALSA_INCLUDE_DIRS})
include_directories(${VORBISFILE_INCLUDE_DIRS})

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

link_libraries(${ALSA_LIBRARY})
link_libraries(${VORBISFILE_LDFLAGS})

add_executable(pxtnplay ${PXTNPLAY_SRC})

