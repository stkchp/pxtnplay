cmake_minimum_required(VERSION 3.5)

find_package(ALSA REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(VORBISFILE REQIRED vorbisfile)

file(GLOB pxtnplay_SRC
	"./src/core/*.h"
	"./src/core/*.cpp"
	"./src/play/*.h"
	"./src/play/*.cpp"
	"./src/pxwr/*.h"
	"./src/pxwr/*.cpp"
	"./src/*.cpp"
	"./src/*.h"
)

add_compile_options(${ALSA_CFLAGS})
add_compile_options(${VORBISFILE_CFLAGS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/play)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/pxwr)
include_directories(${ALSA_INCLUDE_DIRS})
include_directories(${VORBISFILE_INCLUDE_DIRS})

link_libraries(${ALSA_LDFLAGS})
link_libraries(${VORBISFILE_LDFLAGS})

add_executable(pxtnplay ${pxtnplay_SRC})

