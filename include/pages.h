#ifndef PAGES_H
#define PAGES_H

#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "editorfile.h"
#include "library.h"

void display_help();
void display_info();
char file_display();

#endif