cmake_minimum_required(VERSION 3.14)
project(SCE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define option for build type
option(BUILD_TESTING "Build tests" OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)

# Only search for Check if we're building tests
if(BUILD_TESTING)
    pkg_check_modules(CHECK check)
endif()

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

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#   +----------------------------------------+
#   | TODO: separate tests in multiple files |
#   +----------------------------------------+

# Determine build approach based on whether we're testing
if(BUILD_TESTING)
    # Testing mode: Build shared library for tests
    add_library(sce_lib SHARED ${LIB_SOURCES})
    target_link_libraries(sce_lib ${NCURSES_LIBRARIES})
    
    # Main executable links against shared lib
    add_executable(SCE src/main.c)
    target_link_libraries(SCE sce_lib)
    
    # Install the shared lib too
    install(TARGETS sce_lib DESTINATION lib)
else()
    # Normal mode: Build standalone executable with static linking
    add_executable(SCE src/main.c ${LIB_SOURCES})
    target_link_libraries(SCE ${NCURSES_LIBRARIES})
endif()

# Only enable testing if requested and Check is found
if(BUILD_TESTING)
    enable_testing()
    
    if(CHECK_FOUND)
        add_subdirectory(tests)
    else()
        message(WARNING "Check not found, tests will not be built")
    endif()
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt")
    add_subdirectory(benchmarks)
else()
    message(STATUS "Benchmarks directory not set up yet")
endif()

# Install the SCE executable
install(TARGETS SCE DESTINATION bin)