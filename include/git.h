#ifndef GIT_H
#define GIT_H

#include <stdbool.h>

#ifdef _WIN32
    void show_git_wip_window(const char* title);
#endif

char* execute_git_command_in_dir(const char* path, const char* command);
char* execute_git_command(const char* command);
bool is_git_repository(const char* path);
bool is_git_repository_current();
const char* git_get_branch_in_dir(const char* path);
const char* git_get_branch();
void git_parse_status_in_dir(const char* path);
void git_parse_status();
bool git_add_file(const char* filename);
bool git_commit(const char* message);
bool git_push();
bool git_pull();
const char* git_get_file_status(const char* filename);
void git_parse_history_in_dir(const char* path);
void git_parse_history();
char* git_get_commit_details(const char* hash);
void git_history_window();
const char* git_get_repo_name_in_dir(const char* path);
const char* git_get_repo_name();
const char* git_get_user_in_dir(const char* path);
const char* git_get_user();
void git_status_window();
void update_git_status(const char* path);
void update_git_status_current();

#endif