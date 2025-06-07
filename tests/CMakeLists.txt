include_directories(${CHECK_INCLUDE_DIRS} ../src)

add_library(test_support STATIC
    ../src/editorfile.c
    ../src/colors.c
    ../src/variables.c
    ../src/pages.c
    ../src/macros.c
    ../src/library.c
    ../src/cc.c
    ../src/console.c
    ../src/cfiles.c
    ../src/arg.c
    ../src/sceconfig.c
    ../src/git.c
)

add_executable(test_editor test_editor.c)

target_link_libraries(test_editor
    test_support
    ${CHECK_LIBRARIES}
    ${NCURSES_LIBRARIES}
    m
    rt
    pthread
)

add_test(NAME EditorTest COMMAND ${CMAKE_BINARY_DIR}/bin/test_editor)