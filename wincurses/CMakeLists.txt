# wincurses - Windows ncurses compatibility layer
cmake_minimum_required(VERSION 3.14)

if(WIN32)
    # Create wincurses static library
    add_library(wincurses STATIC wincurses.c)
    
    # Link required Windows libraries
    target_link_libraries(wincurses kernel32 user32)
    
    # Set include directories
    target_include_directories(wincurses PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    
    # Install the library (optional)
    install(TARGETS wincurses DESTINATION lib)
    install(FILES wincurses.h DESTINATION include)
else()
    message(STATUS "wincurses skipped - not building on Windows")
endif()