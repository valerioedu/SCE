cmake_minimum_required(VERSION 3.14)
project(SCE)

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    src/main.c
    src/editorfile.c
    src/colors.c
    src/variables.c
    src/pages.c
    src/macros.c
    src/library.c
    src/cc.c
    src/console.c
    src/modules/cfiles.c
    src/arg.c
)

add_executable(SCE ${SOURCES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)
include_directories(${NCURSES_INCLUDE_DIRS})
link_directories(${NCURSES_LIBRARY_DIRS})
target_link_libraries(SCE ${NCURSES_LIBRARIES})

install(TARGETS SCE DESTINATION bin)