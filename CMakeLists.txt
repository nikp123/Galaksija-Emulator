cmake_minimum_required(VERSION 3.0)
project(GalEmu C)

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_SHARED_LIBS OFF)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED sdl2)

add_library(z80 inc/Z80/Z80.c)

add_library(tinyfiledialogs inc/tinyfiledialogs/tinyfiledialogs.c)

add_executable(
	GalEmu WIN32
	src/main.c
	src/z80.c
	src/sdl2_main.c
	src/shared.c
)

add_definitions(${SDL2_CFLAGS_OTHER})
include_directories(${SDL2_INCLUDE_DIRS} inc)
target_link_libraries(GalEmu ${SDL2_LIBRARIES} tinyfiledialogs z80 m)
