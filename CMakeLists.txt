cmake_minimum_required(VERSION 3.14)
project(SCE VERSION 1.0.0 LANGUAGES C)

if (NOT WIN32)
    add_compile_options(
        -Wno-unused-result
        -Wno-format-truncation
        -Wno-zero-length
    )
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build tests" OFF)

if(WIN32)
    set(PDCURSES_DIR ${PDCURSES_DIR} CACHE PATH "Path to PDCursesMod source directory")
    if(NOT EXISTS "${PDCURSES_DIR}/curses.h")
        message(FATAL_ERROR "PDCursesMod not found at specified path: ${PDCURSES_DIR}. Please check the path.")
    endif()
    include_directories(${PDCURSES_DIR})

    file(GLOB PDCURSES_SOURCES
        "${PDCURSES_DIR}/pdcurses/*.c"
        "${PDCURSES_DIR}/wincon/*.c"
    )

    add_compile_definitions(PDCURSES)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(NCURSES REQUIRED ncurses)
    include_directories(${NCURSES_INCLUDE_DIRS})
    link_directories(${NCURSES_LIBRARY_DIRS})
endif()

if(BUILD_TESTING)
    if (NOT WIN32)
        pkg_check_modules(CHECK check)
    endif()
endif()

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

if(BUILD_TESTING)
    add_library(sce_lib SHARED ${LIB_SOURCES})

    if(APPLE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
    endif()

    target_link_libraries(sce_lib ${NCURSES_LIBRARIES})
    
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
    target_link_libraries(SCE ${NCURSES_LIBRARIES})
endif()

target_compile_definitions(SCE PRIVATE "-DPROJECT_VERSION=\"${PROJECT_VERSION}\"")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt")
    add_subdirectory(benchmarks)
else()
    message(STATUS "Benchmarks will be added later")
endif()

install(TARGETS SCE DESTINATION bin)