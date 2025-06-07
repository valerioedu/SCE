#ifndef GIT_H
#define GIT_H

#include <stdbool.h>

// Check if the current directory is a git repository
bool is_git_repository();

// Get the current branch name
const char* git_get_branch();

// Add a file to the git staging area
bool git_add_file(const char* filename);

// Commit changes with the given message
bool git_commit(const char* message);

// Push changes to the remote repository
bool git_push();

// Pull changes from the remote repository
bool git_pull();

// Get the status of a specific file
const char* git_get_file_status(const char* filename);

// Update the internal git status
void git_parse_status();

// Display an interactive git status window
void git_status_window();
const char* git_get_repo_name();
const char* git_get_user();

#endif // GIT_H