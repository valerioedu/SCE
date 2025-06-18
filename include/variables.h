#ifndef VARIABLES_H
#define VARIABLES_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cfiles.h"
#include "library.h"

extern char** variables;
extern size_t variables_count;
extern char **typedefs;
extern size_t typedefs_count;

void save_variables(char* var);
void detect_variables(char* line);
void cleanup_variables();
void init_lines();
void begin_variable_scan();
void finish_variable_scan();

#endif