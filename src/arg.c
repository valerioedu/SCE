#include "arg.h"
#include "library.h"
#include <sys/stat.h>
#include "git.h"

void print_usage() {
    printf("Usage: SCE [OPTIONS] [FILE|DIRECTORY]\n\n");
    printf("Options:\n");
    printf("  -h, --help     Display this help message\n");
    printf("  -v, --version  Display version information\n\n");
    printf("If FILE is provided, SCE will open that file for editing.\n");
    printf("If DIRECTORY is provided, SCE will open the file browser in that location.\n");
    printf("If no arguments are given, SCE will open in the current directory.\n");
}

void print_version() {
    printf("SCE (Simple-Code-Editor) version 1.0\n");
    printf("Copyright (c) 2025\n");
}

bool is_directory(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return false;
    return S_ISDIR(statbuf.st_mode);
}

bool is_file(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return false;
    return S_ISREG(statbuf.st_mode);
}

void args(int argc, char* argv[]) {
    if (argc < 2) return;
    
    for (int i = 1; i < argc; i++) {        
        char resolved_path[MAX_PATH];
            
        if (realpath(argv[i], resolved_path) != NULL) {
            if (is_directory(resolved_path)) {
                strncpy(current_path, resolved_path, MAX_PATH - 1);
                current_path[MAX_PATH - 1] = '\0';
                open_file_browser = true;
                if (is_git_repository(resolved_path))  update_git_status(resolved_path);
                return;
            } else if (is_file(resolved_path)) {
                char file_dir[MAX_PATH];
                strncpy(file_dir, resolved_path, MAX_PATH - 1);
                file_dir[MAX_PATH - 1] = '\0';
                
                char *last_slash = strrchr(file_dir, '/');
                if (last_slash != NULL) {
                    *last_slash = '\0';
                    
                    if (is_git_repository(file_dir)) update_git_status(file_dir);
                }
                load_file(resolved_path);
                return;
            } else {
                printf("Error: '%s' is not a valid file or directory.\n", argv[i]);
                exit(1);
            }
        } else {
            printf("Error: Could not resolve path '%s': %s\n", argv[i], strerror(errno));
            exit(1);
        }
    }
}