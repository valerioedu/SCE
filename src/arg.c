#include "arg.h"
#include "library.h"
#include <sys/stat.h>
#ifdef _WIN32
    #include <Windows.h>
    #include <stdlib.h>
    #include <io.h>
    bool loadfile = true;
#endif
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
    printf("SCE (Simple-Code-Editor) version %s\n", PROJECT_VERSION);
    printf("Copyright (C) 2025 Valerioedu\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software, and you are welcome to redistribute it\n");
    printf("under certain conditions; see the GNU GPL v3 license for details.\n");
}

#ifdef _WIN32
    bool is_directory(const char* path) {
        struct stat local_statbuf;
        if (_stat(path, &local_statbuf) != 0) return false;
        return ((local_statbuf.st_mode & _S_IFDIR) == _S_IFDIR);
    }

    bool is_file(const char* path) {
        struct stat local_statbuf;
        if (_stat(path, &local_statbuf) != 0) return false;
        return ((local_statbuf.st_mode & _S_IFREG) == _S_IFREG);
    }

void args(int argc, char* argv[]) {
    if (argc < 2) return;
    
    for (int i = 1; i < argc; i++) {        
        char resolved_path[MAX_PATH];
        
        if (is_directory(argv[i])) {
            DWORD result = GetFullPathNameA(argv[i], MAX_PATH, resolved_path, NULL);
            if (result == 0 || result >= MAX_PATH) {
                strncpy(resolved_path, argv[i], MAX_PATH - 1);
                resolved_path[MAX_PATH - 1] = '\0';
            }

            strncpy(current_path, resolved_path, MAX_PATH - 1);
            current_path[MAX_PATH - 1] = '\0';
            open_file_browser = true;
            if (is_git_repository(resolved_path)) update_git_status(resolved_path);
            return;
        }
        
        if (is_file(argv[i])) {
            DWORD result = GetFullPathNameA(argv[i], MAX_PATH, resolved_path, NULL);
            if (result == 0 || result >= MAX_PATH) {
                strncpy(resolved_path, argv[i], MAX_PATH - 1);
                resolved_path[MAX_PATH - 1] = '\0';
            }
        } else {
            char current_dir[MAX_PATH];
            char temp_path[MAX_PATH];
            
            if (GetCurrentDirectoryA(MAX_PATH, current_dir) == 0) {
                strcpy(current_dir, ".");
            }
            
            if ((strlen(argv[i]) > 1 && argv[i][1] == ':') || 
                (argv[i][0] == '\\' && argv[i][1] == '\\')) {
                strncpy(resolved_path, argv[i], MAX_PATH - 1);
                resolved_path[MAX_PATH - 1] = '\0';
            } else {
                snprintf(temp_path, MAX_PATH, "%s\\%s", current_dir, argv[i]);
                DWORD result = GetFullPathNameA(temp_path, MAX_PATH, resolved_path, NULL);
                if (result == 0 || result >= MAX_PATH) {
                    strncpy(resolved_path, temp_path, MAX_PATH - 1);
                    resolved_path[MAX_PATH - 1] = '\0';
                }
            }
            
            for (char* p = resolved_path; *p; p++) {
                if (*p == '/') *p = '\\';
            }

            loadfile = false;
        }
        
        char file_dir[MAX_PATH];
        strncpy(file_dir, resolved_path, MAX_PATH - 1);
        file_dir[MAX_PATH - 1] = '\0';
        
        char *last_slash = strrchr(file_dir, '\\');
        if (last_slash != NULL) {
            *last_slash = '\0';
            if (is_git_repository(file_dir)) update_git_status(file_dir);
        }
        
        load_file(resolved_path);
        return;
    }
}
#endif

#ifndef _WIN32
    int confirm_action(const char* message) {
        char response[10];
        printf("%s [y/N]: ", message);
        fflush(stdout);
        
        if (fgets(response, sizeof(response), stdin) == NULL) return 0;
        
        if (response[0] == 'y' || response[0] == 'Y') return 1;
        return 0;
    }

    void uninstall_sce() {
        printf("SCE Editor Uninstaller\n");
        printf("=====================\n\n");
        
        if (!confirm_action("Are you sure you want to uninstall SCE Editor?")) {
            printf("Uninstall cancelled.\n");
            return;
        }
        
        printf("Uninstalling SCE Editor...\n");
        
        int removed_binary = 0;
        const char path[] = "/usr/local/bin/SCE"; 
        struct stat st;
        if (stat(path, &st) == 0) {
            char command[256];
            snprintf(command, sizeof(command), "sudo rm -f \"%s\"", path);
            if (system(command) == 0) {
                printf("Removed: %s\n", path);
                removed_binary = 1;
            } else {
                printf("Failed to remove: %s\n", path);
            }
        }
        
        if (!removed_binary) {
            printf("SCE binary not found in standard locations.\n");
            printf("You may need to remove it manually from your custom installation path.\n");
        }
        
        char config_path[512];
        const char* home = getenv("HOME");
        if (home) {
            snprintf(config_path, sizeof(config_path), "%s/.sceconfig", home);
            
            struct stat st;
            if (stat(config_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                if (confirm_action("Would you like to keep your SCE configuration?")) {
                    printf("Keeping configuration in %s\n", config_path);
                } else {
                    char command[512];
                    snprintf(command, sizeof(command), "rm -rf \"%s\"", config_path);
                    if (system(command) == 0) {
                        printf("Removed configuration directory: %s\n", config_path);
                    } else {
                        printf("Failed to remove configuration directory: %s\n", config_path);
                    }
                }
            } else {
                printf("Configuration directory not found at: %s\n", config_path);
            }
        }
        
        printf("\nSCE Editor has been uninstalled.\n");
        printf("Thank you for using SCE Editor!\n");
    }
    
    bool is_directory(const char* path) {
        struct stat local_statbuf;
        if (stat(path, &local_statbuf) != 0) return false;
        return S_ISDIR(local_statbuf.st_mode);
    }

    bool is_file(const char* path) {
        struct stat local_statbuf;
        if (stat(path, &local_statbuf) != 0) return false;
        return S_ISREG(local_statbuf.st_mode);
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
                    endwin();
                    printf("Error: '%s' is not a valid file or directory.\n", argv[i]);
                    exit(1);
                }
            } 
        }
    }
#endif