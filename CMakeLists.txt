cmake_minimum_required(VERSION 3.0)
project(GalEmu C)

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_SHARED_LIBS OFF)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED sdl2)

add_library(z80 inc/libz80/z80.c)

add_library(tinyfiledialogs inc/tinyfiledialogs/tinyfiledialogs.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

add_executable(
	GalEmu WIN32
	src/main.c
	src/core.c
	src/util/file.c
	src/graphics.c
	src/init.c
)

add_definitions(${SDL2_CFLAGS_OTHER})
include_directories(${SDL2_INCLUDE_DIRS} inc inc/libz80)
target_link_libraries(GalEmu ${SDL2_LIBRARIES} tinyfiledialogs z80 m)
