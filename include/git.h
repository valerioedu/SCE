#ifndef GIT_H
#define GIT_H

#include <stdbool.h>

bool is_git_repository();
const char* git_get_branch();
bool git_add_file(const char* filename);
bool git_commit(const char* message);
bool git_push();
bool git_pull();
const char* git_get_file_status(const char* filename);
void git_parse_status();
void git_status_window();
const char* git_get_repo_name();
const char* git_get_user();

#endif