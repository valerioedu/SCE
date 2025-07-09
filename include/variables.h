#ifndef VARIABLES_H
#define VARIABLES_H

#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cfiles.h"
#include "library.h"

typedef struct {
    char* name;
    int declaration_line;
} Identifier;

extern Identifier* variables;
extern size_t variables_capacity;
extern size_t variables_count;

extern Identifier* typedefs;
extern size_t typedefs_capacity;
extern size_t typedefs_count;

extern int typedef_waiting;

void begin_variable_scan();
void detect_variables(char* line, int line_num);
void cleanup_variables();
void cleanup_typedefs();
void rescan_line_for_declarations(int line_num);

#endif