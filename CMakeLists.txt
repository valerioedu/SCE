cmake_minimum_required(VERSION 3.14)
project(SCE)

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)

pkg_check_modules(CHECK check)

include_directories(include ${NCURSES_INCLUDE_DIRS} src)
link_directories(${NCURSES_LIBRARY_DIRS})

set(LIB_SOURCES
    src/editorfile.c
    src/colors.c
    src/variables.c
    src/pages.c
    src/macros.c
    src/library.c
    src/cc.c
    src/console.c
    src/cfiles.c
    src/arg.c
    src/sceconfig.c
    src/git.c
)

#   +----------------------------------------+
#   | TODO: seperate tests in multiple files |
#   +----------------------------------------+

add_library(sce_lib SHARED ${LIB_SOURCES})
target_link_libraries(sce_lib ${NCURSES_LIBRARIES})

add_executable(SCE src/main.c)
target_link_libraries(SCE sce_lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()

if(CHECK_FOUND)
    add_subdirectory(tests)
else()
    message(WARNING "Check not found, tests will not be built")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt")
    add_subdirectory(benchmarks)
else()
    message(STATUS "Benchmarks directory not set up yet")
endif()

install(TARGETS SCE DESTINATION bin)