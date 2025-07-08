#ifndef EDITORFILE_H
#define EDITORFILE_H

#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "library.h"
#include "variables.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH 1024

extern char current_path[MAX_PATH];

void file_explorer_help();
char* filesystem(const char* dir_path);
void load_file(const char* filepath);
void create_file(const char* current_path);
void create_dir(const char* current_path);
void file_save();
void save_file();
void autosave_file();
void autosaved_load();

#endif