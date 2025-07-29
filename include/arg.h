#ifndef ARG_H
#define ARG_H

#include "library.h"
#include "editorfile.h"

#ifdef _WIN32
    extern bool loadfile;
#else
    void uninstall_sce();
#endif

void args(int argc, char* argv[]);
void print_version();
void print_usage();

#endif