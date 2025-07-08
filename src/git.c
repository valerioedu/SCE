#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
    #include <unistd.h>
#endif

#include "git.h"
#include "macros.h"
#include "library.h"

#define BUFFER_SIZE 1024
#define MAX_COMMAND_LENGTH 2048
#define MAX_BRANCH_LENGTH 128
#define MAX_STATUS_ITEMS 50
#define MAX_COMMITS 100
#define COMMIT_HASH_LENGTH 41
#define COMMIT_MSG_LENGTH 256
#define COMMIT_DATE_LENGTH 32
#define COMMIT_AUTHOR_LENGTH 64
#define MAX_PATH 512

typedef struct {
    char filename[256];
    char status[16];
} GitStatusItem;

typedef struct {
    char hash[COMMIT_HASH_LENGTH];
    char author[COMMIT_AUTHOR_LENGTH];
    char date[COMMIT_DATE_LENGTH];
    char message[COMMIT_MSG_LENGTH];
} GitCommit;

static GitCommit commits[MAX_COMMITS];
static int commit_count = 0;

static GitStatusItem status_items[MAX_STATUS_ITEMS];
static int status_count = 0;
static char current_branch[MAX_BRANCH_LENGTH] = "";

char* execute_git_command_in_dir(const char* path, const char* command) {
    static char buffer[BUFFER_SIZE];
    char full_command[MAX_COMMAND_LENGTH];
    
    snprintf(full_command, MAX_COMMAND_LENGTH, 
             "cd \"%s\" && git %s 2>&1", path, command);
    
    FILE* pipe = popen(full_command, "r");
    if (!pipe) return "ERROR: Git command execution failed";
    
    char* result = fgets(buffer, BUFFER_SIZE, pipe);
    pclose(pipe);
    
    if (result == NULL) return "";
    
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
    
    return buffer;
}

char* execute_git_command(const char* command) {
    static char buffer[BUFFER_SIZE];
    char full_command[MAX_COMMAND_LENGTH];
    
    snprintf(full_command, MAX_COMMAND_LENGTH, "git %s 2>&1", command);
    
    FILE* pipe = popen(full_command, "r");
    if (!pipe) return "ERROR: Git command execution failed";
    
    char* result = fgets(buffer, BUFFER_SIZE, pipe);
    pclose(pipe);
    
    if (result == NULL) return "";
    
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
    
    return buffer;
}

bool is_git_repository(const char* path) {
    if (in_memory) return false;
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, MAX_COMMAND_LENGTH, 
             "cd \"%s\" && git rev-parse --is-inside-work-tree > /dev/null 2>&1", path);
    return system(command) == 0;
}

bool is_git_repository_current() {
    return system("git rev-parse --is-inside-work-tree > /dev/null 2>&1") == 0;
}

const char* git_get_branch_in_dir(const char* path) {
    static char branch[MAX_BRANCH_LENGTH];
    
    if (!is_git_repository(path)) {
        strcpy(branch, "Not a git repository");
        return branch;
    }
    
    char* result = execute_git_command_in_dir(path, "rev-parse --abbrev-ref HEAD");
    if (result && strlen(result) > 0) {
        strncpy(branch, result, MAX_BRANCH_LENGTH - 1);
        branch[MAX_BRANCH_LENGTH - 1] = '\0';
    } else {
        strcpy(branch, "unknown");
    }
    
    strncpy(current_branch, branch, MAX_BRANCH_LENGTH - 1);
    current_branch[MAX_BRANCH_LENGTH - 1] = '\0';
    
    return branch;
}

const char* git_get_branch() {
    if (!is_git_repository_current()) {
        strcpy(current_branch, "Not a git repository");
        return current_branch;
    }
    
    char* result = execute_git_command("rev-parse --abbrev-ref HEAD");
    if (result && strlen(result) > 0) {
        strncpy(current_branch, result, MAX_BRANCH_LENGTH - 1);
        current_branch[MAX_BRANCH_LENGTH - 1] = '\0';
    } else strcpy(current_branch, "unknown");
    
    return current_branch;
}

void git_parse_status_in_dir(const char* path) {
    if (!is_git_repository(path)) {
        status_count = 0;
        return;
    }
    
    char line[BUFFER_SIZE];
    status_count = 0;
    
    FILE* pipe = popen(execute_git_command_in_dir(path, "status --porcelain"), "r");
    if (!pipe) return;
    
    while (fgets(line, BUFFER_SIZE, pipe) && status_count < MAX_STATUS_ITEMS) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        
        if (len < 3) continue;
        
        char status_code[3] = {line[0], line[1], '\0'};
        strncpy(status_items[status_count].status, status_code, sizeof(status_items[status_count].status) - 1);
        status_items[status_count].status[sizeof(status_items[status_count].status) - 1] = '\0';
        
        int filename_start = 2;
        while (filename_start < len && line[filename_start] == ' ')
            filename_start++;
        
        strncpy(status_items[status_count].filename, &line[filename_start], 
                sizeof(status_items[status_count].filename) - 1);
        status_items[status_count].filename[sizeof(status_items[status_count].filename) - 1] = '\0';
        
        status_count++;
    }
    
    pclose(pipe);
}

void git_parse_status() {
    if (!is_git_repository_current()) {
        status_count = 0;
        return;
    }
    
    char command[MAX_COMMAND_LENGTH];
    FILE* pipe;
    char line[BUFFER_SIZE];
    
    status_count = 0;
    
    strcpy(command, "git status --porcelain");
    pipe = popen(command, "r");
    if (!pipe) return;
    
    while (fgets(line, BUFFER_SIZE, pipe) && status_count < MAX_STATUS_ITEMS) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        
        if (len < 3) continue;
        
        char status_code[3] = {line[0], line[1], '\0'};
        strncpy(status_items[status_count].status, status_code, sizeof(status_items[status_count].status) - 1);
        status_items[status_count].status[sizeof(status_items[status_count].status) - 1] = '\0';
        
        int filename_start = 2;
        while (filename_start < len && line[filename_start] == ' ')
            filename_start++;
        
        strncpy(status_items[status_count].filename, &line[filename_start], 
                sizeof(status_items[status_count].filename) - 1);
        status_items[status_count].filename[sizeof(status_items[status_count].filename) - 1] = '\0';
        
        status_count++;
    }
    
    pclose(pipe);
}

bool git_add_file(const char* filename) {
    if (!is_git_repository_current()) return false;
    
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, MAX_COMMAND_LENGTH, "add \"%s\"", filename);
    
    char* result = execute_git_command(command);
    return (result != NULL && strstr(result, "error") == NULL);
}

bool git_commit(const char* message) {
    if (!is_git_repository_current()) return false;
    
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, MAX_COMMAND_LENGTH, "commit -m \"%s\"", message);
    
    char* result = execute_git_command(command);
    return (result != NULL && strstr(result, "error") == NULL);
}

bool git_push() {
    if (!is_git_repository_current()) return false;

    return system("git push 2> /dev/null") == 0;
}

bool git_pull() {
    if (!is_git_repository_current()) return false;

    return system("git pull 2> /dev/null") == 0;
}

const char* git_get_file_status(const char* filename) {
    for (int i = 0; i < status_count; i++) {
        if (strcmp(status_items[i].filename, filename) == 0) {
            return status_items[i].status;
        }
    }
    
    return "";
}

void git_parse_history_in_dir(const char* path) {
    if (!is_git_repository(path)) {
        commit_count = 0;
        return;
    }
    
    char line[BUFFER_SIZE];
    commit_count = 0;
    
    FILE* pipe = popen(execute_git_command_in_dir(path, "log --pretty=format:\"%H|%an|%ad|%s\" --date=short"), "r");
    if (!pipe) return;
    
    while (fgets(line, BUFFER_SIZE, pipe) && commit_count < MAX_COMMITS) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        
        char* hash = strtok(line, "|");
        char* author = strtok(NULL, "|");
        char* date = strtok(NULL, "|");
        char* message = strtok(NULL, "|");
        
        if (hash && author && date && message) {
            strncpy(commits[commit_count].hash, hash, COMMIT_HASH_LENGTH - 1);
            commits[commit_count].hash[COMMIT_HASH_LENGTH - 1] = '\0';
            
            strncpy(commits[commit_count].author, author, COMMIT_AUTHOR_LENGTH - 1);
            commits[commit_count].author[COMMIT_AUTHOR_LENGTH - 1] = '\0';
            
            strncpy(commits[commit_count].date, date, COMMIT_DATE_LENGTH - 1);
            commits[commit_count].date[COMMIT_DATE_LENGTH - 1] = '\0';
            
            strncpy(commits[commit_count].message, message, COMMIT_MSG_LENGTH - 1);
            commits[commit_count].message[COMMIT_MSG_LENGTH - 1] = '\0';
            
            commit_count++;
        }
    }
    
    pclose(pipe);
}

void git_parse_history() {
    if (!is_git_repository_current()) {
        commit_count = 0;
        return;
    }
    
    char command[MAX_COMMAND_LENGTH];
    FILE* pipe;
    char line[BUFFER_SIZE];
    char buffer[BUFFER_SIZE * 4] = {0};
    
    commit_count = 0;
    
    strcpy(command, "git log --pretty=format:\"%H|%an|%ad|%s\" --date=short");
    pipe = popen(command, "r");
    if (!pipe) return;
    
    while (fgets(line, BUFFER_SIZE, pipe) && commit_count < MAX_COMMITS) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        
        char* hash = strtok(line, "|");
        char* author = strtok(NULL, "|");
        char* date = strtok(NULL, "|");
        char* message = strtok(NULL, "|");
        
        if (hash && author && date && message) {
            strncpy(commits[commit_count].hash, hash, COMMIT_HASH_LENGTH - 1);
            commits[commit_count].hash[COMMIT_HASH_LENGTH - 1] = '\0';
            
            strncpy(commits[commit_count].author, author, COMMIT_AUTHOR_LENGTH - 1);
            commits[commit_count].author[COMMIT_AUTHOR_LENGTH - 1] = '\0';
            
            strncpy(commits[commit_count].date, date, COMMIT_DATE_LENGTH - 1);
            commits[commit_count].date[COMMIT_DATE_LENGTH - 1] = '\0';
            
            strncpy(commits[commit_count].message, message, COMMIT_MSG_LENGTH - 1);
            commits[commit_count].message[COMMIT_MSG_LENGTH - 1] = '\0';
            
            commit_count++;
        }
    }
    
    pclose(pipe);
}

char* git_get_commit_details(const char* hash) {
    static char details[BUFFER_SIZE * 10];
    char command[MAX_COMMAND_LENGTH];
    FILE* pipe;
    char line[BUFFER_SIZE];
    
    snprintf(command, MAX_COMMAND_LENGTH, 
             "git show --stat --format=\"Commit: %%H%%nAuthor: %%an %%ae%%nDate:   %%ad%%nMessage: %%s%%n%%n\" %s", 
             hash);
    
    pipe = popen(command, "r");
    if (!pipe) return "Error getting commit details";
    
    details[0] = '\0';
    
    while (fgets(line, BUFFER_SIZE, pipe)) {
        if (strlen(details) + strlen(line) < sizeof(details) - 1) {
            strcat(details, line);
        } else {
            strcat(details, "...[truncated]");
            break;
        }
    }
    
    pclose(pipe);
    return details;
}

void git_history_window() {
    WINDOW* win;
    int ch, selected = 0;
    int start_row = 0;
    
    git_parse_history();
    
    win = newwin(LINES - 4, COLS - 4, 2, 2);
    keypad(win, TRUE);
    
    while (1) {
        wclear(win);
        box(win, 0, 0);
        
        mvwprintw(win, 0, (COLS - 14) / 2, " Git History ");
        
        mvwprintw(win, LINES - 6, 2, "↑/↓: Navigate | D: View Details | C: Checkout | Q: Quit");
        
        if (commit_count == 0) mvwprintw(win, 2, 2, "No commits found");
        else {
            int max_display = LINES - 8;
            
            if (selected < start_row) start_row = selected;
            else if (selected >= start_row + max_display) start_row = selected - max_display + 1;
            
            wattron(win, A_BOLD);
            mvwprintw(win, 1, 2, "%-10s %-20s %-50s", "Date", "Author", "Message");
            wattroff(win, A_BOLD);
            
            for (int i = start_row; i < commit_count && i < start_row + max_display; i++) {
                if (i == selected) wattron(win, A_REVERSE);
                
                char truncated_msg[51] = {0};
                strncpy(truncated_msg, commits[i].message, 50);
                truncated_msg[50] = '\0';
                
                mvwprintw(win, 2 + i - start_row, 2, "%-10s %-20s %-50s", 
                         commits[i].date, 
                         commits[i].author,
                         truncated_msg);
                
                if (i == selected) wattroff(win, A_REVERSE);
            }
        }
        
        wrefresh(win);
        
        ch = wgetch(win);
        
        switch (ch) {
            case KEY_UP:
                if (selected > 0) selected--;
                break;
                
            case KEY_DOWN:
                if (selected < commit_count - 1) selected++;
                break;
                
            case 'd':
            case 'D':
                if (commit_count > 0) {
                    WINDOW* details_win = newwin(LINES - 8, COLS - 8, 4, 4);
                    keypad(details_win, TRUE);
                    
                    char* details = git_get_commit_details(commits[selected].hash);
                    
                    while (1) {
                        wclear(details_win);
                        box(details_win, 0, 0);
                        
                        mvwprintw(details_win, 0, (COLS - 20) / 2, " Commit Details ");
                        
                        int line = 1;
                        int col = 2;
                        char* p = details;
                        
                        while (*p && line < LINES - 10) {
                            if (*p == '\n') {
                                wmove(details_win, line++, 2);
                                col = 2;
                            } else {
                                mvwaddch(details_win, line, col++, *p);
                                
                                if (col >= COLS - 10) {
                                    line++;
                                    col = 2;
                                }
                            }
                            p++;
                        }
                        
                        mvwprintw(details_win, LINES - 10, 2, "Press any key to return");
                        
                        wrefresh(details_win);
                        
                        if (wgetch(details_win) != ERR) break;
                    }
                    
                    delwin(details_win);
                    
                    wclear(win);
                    box(win, 0, 0);
                    wrefresh(win);
                }
                break;
                
            case 'c':
            case 'C':
                if (commit_count > 0) {
                    mvwprintw(win, LINES - 8, 2, "Checkout commit %s? (y/n): ", commits[selected].hash);
                    wrefresh(win);
                    
                    ch = wgetch(win);
                    
                    if (ch == 'y' || ch == 'Y') {
                        char command[MAX_COMMAND_LENGTH];
                        snprintf(command, MAX_COMMAND_LENGTH, "checkout %s", commits[selected].hash);
                        
                        char* result = execute_git_command(command);
                        
                        mvwprintw(win, LINES - 8, 2, "%-60s", "");
                        
                        if (strstr(result, "error") == NULL) mvwprintw(win, LINES - 8, 2, "Checked out commit %s", commits[selected].hash);
                        else mvwprintw(win, LINES - 8, 2, "Failed to checkout: %s", result);
                    } 
                    else  mvwprintw(win, LINES - 8, 2, "%-60s", "");

                    wrefresh(win);
                    napms(1500);
                }
                break;
                
            case 'q':
            case 'Q':
            case 27:
                delwin(win);
                return;
        }
    }
}

const char* git_get_repo_name_in_dir(const char* path) {
    static char repo_name[256] = {0};
    
    if (!is_git_repository(path)) return "";
    
    char* result = execute_git_command_in_dir(path, "rev-parse --show-toplevel");
    
    if (result && strlen(result) > 0) {
        char* last_slash = strrchr(result, '/');
        if (last_slash) {
            strncpy(repo_name, last_slash + 1, sizeof(repo_name) - 1);
            repo_name[sizeof(repo_name) - 1] = '\0';
        } else {
            strncpy(repo_name, result, sizeof(repo_name) - 1);
            repo_name[sizeof(repo_name) - 1] = '\0';
        }
    } else return "";
    
    return repo_name;
}

const char* git_get_repo_name() {
    static char repo_name[256] = {0};
    
    if (!is_git_repository_current()) return "";
    
    char* result = execute_git_command("rev-parse --show-toplevel");
    
    if (result && strlen(result) > 0) {
        char* last_slash = strrchr(result, '/');
        if (last_slash) {
            strncpy(repo_name, last_slash + 1, sizeof(repo_name) - 1);
            repo_name[sizeof(repo_name) - 1] = '\0';
        } else {
            strncpy(repo_name, result, sizeof(repo_name) - 1);
            repo_name[sizeof(repo_name) - 1] = '\0';
        }
    } else return "";
    
    return repo_name;
}

const char* git_get_user_in_dir(const char* path) {
    static char user_name[256] = {0};
    
    if (!is_git_repository(path)) return "";
    
    char* result = execute_git_command_in_dir(path, "config user.name");
    
    if (result && strlen(result) > 0) {
        strncpy(user_name, result, sizeof(user_name) - 1);
        user_name[sizeof(user_name) - 1] = '\0';
    } else return "";
    
    return user_name;
}

const char* git_get_user() {
    static char user_name[256] = {0};
    
    if (!is_git_repository_current()) return "";
    
    char* result = execute_git_command("config user.name");
    
    if (result && strlen(result) > 0) {
        strncpy(user_name, result, sizeof(user_name) - 1);
        user_name[sizeof(user_name) - 1] = '\0';
    } else return "";
    
    return user_name;
}

void git_status_window() {
    WINDOW* win;
    int ch, selected = 0;
    
    git_parse_status();
    
    win = newwin(LINES - 4, COLS - 4, 2, 2);
    keypad(win, TRUE);
    
    while (1) {
        wclear(win);
        box(win, 0, 0);
        
        mvwprintw(win, 0, (COLS - 16) / 2, " Git Status - %s ", git_get_branch());
        
        mvwprintw(win, LINES - 6, 2, "A: Add file | C: Commit | P: Push | L: Pull | H: History | Q: Quit");
        
        if (status_count == 0) {
            mvwprintw(win, 2, 2, "No changes detected");
        } else {
            for (int i = 0; i < status_count && i < LINES - 8; i++) {
                if (i == selected) {
                    wattron(win, A_REVERSE);
                }
                
                char status_desc[32];
                if (strncmp(status_items[i].status, "M", 1) == 0) {
                    strcpy(status_desc, "Modified");
                } else if (strncmp(status_items[i].status, "A", 1) == 0) {
                    strcpy(status_desc, "Added");
                } else if (strncmp(status_items[i].status, "D", 1) == 0) {
                    strcpy(status_desc, "Deleted");
                } else if (strncmp(status_items[i].status, "R", 1) == 0) {
                    strcpy(status_desc, "Renamed");
                } else if (strncmp(status_items[i].status, "C", 1) == 0) {
                    strcpy(status_desc, "Copied");
                } else if (strncmp(status_items[i].status, "U", 1) == 0) {
                    strcpy(status_desc, "Updated");
                } else if (strncmp(status_items[i].status, "??", 2) == 0) {
                    strcpy(status_desc, "Untracked");
                } else {
                    strcpy(status_desc, status_items[i].status);
                }
                
                mvwprintw(win, i + 2, 2, "%-10s %s", status_desc, status_items[i].filename);
                
                if (i == selected) {
                    wattroff(win, A_REVERSE);
                }
            }
        }
        
        wrefresh(win);
        
        ch = wgetch(win);
        
        switch (ch) {
            case KEY_UP:
                if (selected > 0) selected--;
                break;
                
            case KEY_DOWN:
                if (selected < status_count - 1) selected++;
                break;
                
            case 'a':
            case 'A':
                if (status_count > 0) {
                    if (git_add_file(status_items[selected].filename)) {
                        mvwprintw(win, LINES - 8, 2, "Added %s", status_items[selected].filename);
                    } else {
                        mvwprintw(win, LINES - 8, 2, "Failed to add %s", status_items[selected].filename);
                    }
                    wrefresh(win);
                    napms(1500);
                    git_parse_status();
                }
                break;
                
            case 'c':
            case 'C':
                {
                    for (int i = 0; i < COLS - 8; i++) {
                        mvwprintw(win, LINES - 8, 2 + i, " ");
                    }
                    
                    mvwprintw(win, LINES - 8, 2, "Commit message: ");
                    echo();
                    char message[256];
                    wgetnstr(win, message, 255);
                    noecho();
                    
                    if (strlen(message) > 0) {
                        if (git_commit(message)) {
                            mvwprintw(win, LINES - 8, 2, "Changes committed successfully");
                        } else {
                            mvwprintw(win, LINES - 8, 2, "Failed to commit changes");
                        }
                        wrefresh(win);
                        napms(1500);
                        git_parse_status();
                    }
                }
                break;
                
            case 'p':
            case 'P':
                if (git_push()) {
                    mvwprintw(win, LINES - 8, 2, "Changes pushed successfully");
                } else {
                    mvwprintw(win, LINES - 8, 2, "Failed to push changes");
                }
                wrefresh(win);
                napms(1500);
                break;
                
            case 'l':
            case 'L':
                if (git_pull()) {
                    mvwprintw(win, LINES - 8, 2, "Successfully pulled changes");
                } else {
                    mvwprintw(win, LINES - 8, 2, "Failed to pull changes");
                }
                wrefresh(win);
                napms(1500);
                break;
                
            case 'q':
            case 'Q':
            case 27: {
                int max_y, max_x;
                getmaxyx(win, max_y, max_x);
                mvwprintw(win, max_y / 2, (max_x - strlen("Press Enter")) / 2, "Press Enter");
                wrefresh(win);
                delwin(win);
                return;
                }
            case 'h':
            case 'H':
                git_history_window();
                wclear(win);
                box(win, 0, 0);
                git_parse_status();
                break;
        }
    }
}

void update_git_status(const char* path) {
    if (!is_git_repository(path)) return;
    
    char cwd[MAX_PATH];
    getcwd(cwd, MAX_PATH);
    
    chdir(path);
    git_get_branch();
    git_parse_status();
    git_get_repo_name();
    git_get_user();
    chdir(cwd);
}

void update_git_status_current() {
    if (!is_git_repository_current()) return;
    
    git_get_branch();
    git_parse_status();
    git_get_repo_name();
    git_get_user();
}