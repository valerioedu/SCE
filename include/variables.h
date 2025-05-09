#ifndef VARIABLES_H
#define VARIABLES_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cfiles.h"
#include "library.h"

#define MAX_VARIABLES 128*128

extern char* variables[MAX_VARIABLES];

void save_variables(char* var);
void detect_variables(char* line);

#endif