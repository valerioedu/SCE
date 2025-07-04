cmake_minimum_required(VERSION 3.14)
project(SCE C)

if(MSVC)
    set(WIN32 TRUE)
endif()

add_compile_options(
    -Wno-unused-result
    -Wno-format-truncation
    -Wno-zero-length
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build tests" OFF)

# Platform-specific dependencies
if(WIN32)
    message(STATUS "Building for Windows - using wincurses")
    set(CURSES_LIBRARIES "")
    set(CURSES_INCLUDE_DIRS "")
    add_subdirectory(wincurses)
    set(CURSES_LIBRARIES wincurses)
    set(CURSES_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/wincurses)
else()
    message(STATUS "Building for Unix-like system - using ncurses")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(NCURSES REQUIRED ncurses)
    set(CURSES_LIBRARIES ${NCURSES_LIBRARIES})
    set(CURSES_INCLUDE_DIRS ${NCURSES_INCLUDE_DIRS})
    link_directories(${NCURSES_LIBRARY_DIRS})
endif()

if(BUILD_TESTING)
    if(NOT WIN32)
        pkg_check_modules(CHECK check)
    else()
        # On Windows, we'll skip Check framework for now
        message(WARNING "Check framework not configured for Windows builds")
    endif()
endif()

include_directories(include ${CURSES_INCLUDE_DIRS} src)

set(LIB_SOURCES
    src/editorfile.c
    src/colors.c
    src/variables.c
    src/pages.c
    src/macros.c
    src/library.c
    src/cc.c
    src/console.c
    src/arg.c
    src/sceconfig.c
    src/git.c
)

#   +----------------------------------------+
#   | TODO: separate tests in multiple files |
#   +----------------------------------------+

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(BUILD_TESTING AND NOT WIN32)
    add_library(sce_lib SHARED ${LIB_SOURCES})

    if(APPLE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
    endif()

    target_link_libraries(sce_lib ${CURSES_LIBRARIES})
    
    add_executable(SCE src/main.c)
    target_link_libraries(SCE sce_lib)
    
    enable_testing()
    
    if(CHECK_FOUND)
        add_subdirectory(tests)
    else()
        message(WARNING "Check not found, tests will not be built")
    endif()
    
    install(TARGETS sce_lib DESTINATION lib)
else()
    add_executable(SCE src/main.c ${LIB_SOURCES})
    target_link_libraries(SCE ${CURSES_LIBRARIES})
    
    # Windows-specific libraries
    if(WIN32)
        target_link_libraries(SCE kernel32 user32)
    endif()
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt")
    add_subdirectory(benchmarks)
else()
    message(STATUS "Benchmarks will be added later")
endif()

install(TARGETS SCE DESTINATION bin)