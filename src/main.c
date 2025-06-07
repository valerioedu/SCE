#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "library.h"
#include "macros.h"
#include "pages.h"
#include "cc.h"
#include "colors.h"
#include "console.h"
#include "variables.h"
#include "editorfile.h"
#include "arg.h"
#include "sceconfig.h"
#include "git.h"

EditorConfig config = {0};
int horizontal_offset = 0;

UndoState undo_history[MAX_UNDO] = {0};
int undo_count = 0;
int undo_position = 0;
char** lines = NULL;
size_t lines_capacity = 0;
size_t count = 1;
int line_count = 1;
int current_line = 0;
int current_col = 0;
int start_line = 0;
char file_name[512] = {0};
char text[MAX_LINES * MAX_COLS] = {0};

void init_lines() {
    lines_capacity = 100;  // Initial capacity of 100 lines
    lines = malloc(lines_capacity * sizeof(char*));
    if (!lines) {
        endwin();
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    for (size_t i = 0; i < lines_capacity; i++) {
        lines[i] = malloc(MAX_COLS);
        if (!lines[i]) {
            endwin();
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        lines[i][0] = '\0';
    }
}

void ensure_lines_capacity(size_t needed_lines) {
    if (needed_lines <= lines_capacity) return;
    
    size_t new_capacity = lines_capacity * 2;
    while (new_capacity < needed_lines) new_capacity *= 2;
    
    char **new_lines = realloc(lines, new_capacity * sizeof(char*));
    if (!new_lines) return;
    
    lines = new_lines;
    
    for (size_t i = lines_capacity; i < new_capacity; i++) {
        lines[i] = malloc(MAX_COLS);
        if (!lines[i]) return;
        lines[i][0] = '\0';
    }
    
    lines_capacity = new_capacity;
}

void cleanup_lines() {
    if (!lines) return;
    
    for (size_t i = 0; i < lines_capacity; i++) {
        free(lines[i]);
    }
    free(lines);
    lines = NULL;
}

void update_screen_content(int start_line) {
    int row, col;
    getmaxyx(stdscr, row, col);

    inside_multiline_comment = 0;
    
    begin_variable_scan();

    for (int i = 0; i < line_count; i++) {
        detect_variables(lines[i]);
    }

    inside_multiline_comment = 0;
    
    for (int i = 0; i < line_count; i++) {
        check_variables(lines[i]);
    }

    for (int i = 0; i < row - 1; i++) {
        move(i, 0);
        clrtoeol();
        if (i + start_line < line_count) {
            mvprintw(i, 0, "%4d: ", i + start_line + 1);
        }
    }

    int end_line = line_count - 1;
    int line_offset = 6;
    int display_width = col - line_offset;

    for (int i = start_line, screen_line = 0; i <= end_line; i++, screen_line++) {
        mvprintw(screen_line, 0, "%4d: ", i + 1);
        
        KeywordInfo blue_info = check_blue_keywords(lines[i]);
        KeywordInfo purple_info = check_purple_keywords(lines[i]);
        KeywordInfo function_info = check_functions(lines[i]);
        KeywordInfo parentheses_info = color_parentheses(lines[i]);
        KeywordInfo variable_info = check_variables(lines[i]);
        KeywordInfo quotes_info = color_quotes(lines[i]);
        KeywordInfo comments_info = color_comments(lines[i]);

        for (int j = horizontal_offset; j < strlen(lines[i]) && j - horizontal_offset < display_width; j++) {
            move(screen_line, line_offset + j - horizontal_offset);
            
            bool colored = false;
            
            for (int k = 0; k < comments_info.count; k++) {
                if (j >= comments_info.keywords[k].start && j < comments_info.keywords[k].end) {
                    attron(COLOR_PAIR(7));
                    colored = true;
                    break;
                }
            }
            
            if (!colored) {
                if (!colored) {
                    for (int k = 0; k < quotes_info.count; k++) {
                        if (j >= quotes_info.keywords[k].start && j < quotes_info.keywords[k].end) {
                            attron(COLOR_PAIR(6));
                            colored = true;
                            break;
                        }
                    }
                }
                
                if (!colored) {
                    for (int k = 0; k < blue_info.count; k++) {
                        if (j >= blue_info.keywords[k].start && j < blue_info.keywords[k].end) {
                            attron(COLOR_PAIR(1));
                            colored = true;
                            break;
                        }
                    }
                }
                
                if (!colored) {
                    for (int k = 0; k < purple_info.count; k++) {
                        if (j >= purple_info.keywords[k].start && j < purple_info.keywords[k].end) {
                            attron(COLOR_PAIR(2));
                            colored = true;
                            break;
                        }
                    }
                }
                
                if (!colored) {
                    for (int k = 0; k < function_info.count; k++) {
                        if (j >= function_info.keywords[k].start && j < function_info.keywords[k].end) {
                            attron(COLOR_PAIR(3));
                            colored = true;
                            break;
                        }
                    }
                }
                
                if (!colored) {
                    for (int k = 0; k < parentheses_info.count; k++) {
                        if (j >= parentheses_info.keywords[k].start && j < parentheses_info.keywords[k].end) {
                            attron(COLOR_PAIR(4));
                            colored = true;
                            break;
                        }
                    }
                }
                
                if (!colored) {
                    for (int k = 0; k < variable_info.count; k++) {
                        if (j >= variable_info.keywords[k].start && j < variable_info.keywords[k].end) {
                            attron(COLOR_PAIR(5));
                            colored = true;
                            break;
                        }
                    }
                }
            }
            
            addch(lines[i][j]);

            if (colored) {
                attroff(COLOR_PAIR(1)); // Blue keywords
                attroff(COLOR_PAIR(2)); // Purple keywords
                attroff(COLOR_PAIR(3)); // Functions
                attroff(COLOR_PAIR(4)); // Parentheses
                attroff(COLOR_PAIR(5)); // Variables
                attroff(COLOR_PAIR(6)); // Strings
                attroff(COLOR_PAIR(7)); // Comments
                attroff(COLOR_PAIR(8)); // Typedefs
            }
        }
    }
    finish_variable_scan();
}

void update_status_bar() {
    int row, col;
    getmaxyx(stdscr, row, col);
    
    move(row - 1, 0);
    clrtoeol();
    
    for (int i = 0; i < col; i++) {
        mvaddch(row - 1, i, ' ');
    }
    
    mvprintw(row - 1, 2, "Line: %d, Column: %d", current_line + 1, current_col + 1);

    char file_dir[MAX_PATH] = {0};
    if (file_name[0] != '\0') {
        strncpy(file_dir, file_name, sizeof(file_dir) - 1);
        char *last_slash = strrchr(file_dir, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';
        }
    } else {
        getcwd(file_dir, MAX_PATH);
    }

    if (is_git_repository(file_dir)) {
        const char* repo_name = git_get_repo_name_in_dir(file_dir);
        const char* branch = git_get_branch_in_dir(file_dir);
        const char* user = git_get_user_in_dir(file_dir);
        
        int git_info_len = strlen(repo_name) + strlen(branch) + strlen(user) + 8;
        int git_pos = (col - git_info_len) / 2;
        if (git_pos < 30) git_pos = 30;
        
        attron(A_BOLD);
        mvprintw(row - 1, git_pos, "[%s:%s@%s]", repo_name, branch, user);
        attroff(A_BOLD);
    }

    if (file_name[0] != '\0') {
        char display_name[64] = {0};
        char* basename = strrchr(file_name, '/');
        if (basename) {
            strncpy(display_name, basename + 1, sizeof(display_name) - 1);
        } else {
            strncpy(display_name, file_name, sizeof(display_name) - 1);
        }
        mvprintw(row - 1, col - strlen(display_name) - 6, "File: %s", display_name);
    }
}

void display_lines() {
    clear();
    int row, col;
    getmaxyx(stdscr, row, col);

    int line_offset = 6;
    int max_display_lines = row - 1;

    int start_line = 0;
    int end_line = line_count - 1;

    if (line_count > max_display_lines) {
        if (current_line <= max_display_lines / 2) {
            start_line = 0;
            end_line = max_display_lines - 1;
        } else if (current_line >= line_count - max_display_lines / 2) {
            start_line = line_count - max_display_lines;
            end_line = line_count - 1;
        } else {
            start_line = current_line - max_display_lines / 2;
            end_line = current_line + max_display_lines / 2 - 1;
        }
    }

    move(row - 1, 0);
    clrtoeol();

    update_status_bar();
}

void apply_resize() {
    timeout(0);
    if (resizeterm(0, 0) == ERR) return;
    clear();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int max_display_lines = rows - 1;
    int start_line = 0;

    if (line_count > max_display_lines) {
        if (current_line <= max_display_lines / 2) start_line = 0;
        else if (current_line >= line_count - max_display_lines / 2) start_line = line_count - max_display_lines;
        else start_line = current_line - max_display_lines / 2;
    }

    int visible_cols = cols - 6;

    if (current_col < horizontal_offset) horizontal_offset = current_col;
    else if (current_col >= horizontal_offset + visible_cols) horizontal_offset = current_col - visible_cols + 1;

    update_screen_content(start_line);
    update_status_bar();

    move(current_line - start_line, 6 + current_col - horizontal_offset);

    refresh();
}

void editor() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int c = getch();
    bool need_redraw = false;
    int row, col;
    getmaxyx(stdscr, row, col);
    
    switch (c) {
        case KEY_DC:
            save_undo_state();
            if (current_col < strlen(lines[current_line])) {
                memmove(&lines[current_line][current_col], 
                    &lines[current_line][current_col + 1], 
                    strlen(lines[current_line]) - current_col + 1);
                    need_redraw = true;
            } else if (current_line < line_count - 1) {
                if (strlen(lines[current_line]) + strlen(lines[current_line + 1]) < MAX_COLS) {
                    strcat(lines[current_line], lines[current_line + 1]);

                    for (int i = current_line + 1; i < line_count - 1; i++) {
                        strcpy(lines[i], lines[i + 1]);
                    }

                    line_count--;
                    need_redraw = true;
                }
            }
            break;
        case KEY_F(4):
            if (file_name[0] != '\0') {
                save_file();
            } else {
                file_save();
            }
            need_redraw = true;
            break;
        case KEY_F(3):
            filesystem(NULL);
            need_redraw = true;
            break;
        case '\t':
            tab();
            need_redraw = true;
            break;
        case ESCAPE:
            endwin();
            exit(0);
            break;
        case KEY_F(1):
            display_help();
            need_redraw = true;
            break;
        case KEY_F(2):
            display_info();
            need_redraw = true;
            break;
        case KEY_UP:
            if (current_line > 0) {
                current_line--;
                if (current_col > strlen(lines[current_line])) current_col = strlen(lines[current_line]);
                need_redraw = true;
            }
            break;
        case KEY_DOWN:
            if (current_line < line_count - 1) {
                current_line++;
                if (current_col > strlen(lines[current_line])) current_col = strlen(lines[current_line]);
                need_redraw = true;
            }
            break;
        case KEY_LEFT:
            if (current_col > 0) current_col--;
            else if (current_col == 0 && current_line > 0) {
                current_line--;
                current_col = strlen(lines[current_line]);
                need_redraw = true;
            }
            break;
        case KEY_RIGHT:
            if (current_col < strlen(lines[current_line])) current_col++;
            else if (current_col == strlen(lines[current_line]) && current_line < line_count - 1) {
                current_line++;
                current_col = 0;
                need_redraw = true;
            }
            break;
        case '\n':
            save_undo_state();
            ensure_lines_capacity(line_count + 1);
            bool between_braces = false;
            int indent = 0;

            while (indent < strlen(lines[current_line]) && lines[current_line][indent] == ' ') {
                indent++;
            }

            if (current_col > 0 && current_col < strlen(lines[current_line])) {
                if (lines[current_line][current_col - 1] == '{' && lines[current_line][current_col] == '}') {
                    between_braces = true;
                }
            }

            for (int i = line_count; i > current_line + 1; i--) {
                strcpy(lines[i], lines[i-1]);
            }
            
            strncpy(lines[current_line + 1], &lines[current_line][current_col], MAX_COLS - 1);
            lines[current_line + 1][MAX_COLS - 1] = '\0';
            lines[current_line][current_col] = '\0';
            
            line_count++;
            current_line++;

            if (between_braces) {
                char temp[MAX_COLS];
                snprintf(temp, MAX_COLS, "%*s", indent + 4, "");
                strcpy(lines[current_line], temp);
                
                ensure_lines_capacity(line_count + 1);
                for (int i = line_count; i > current_line + 1; i--) {
                    strcpy(lines[i], lines[i-1]);
                }
                
                snprintf(temp, MAX_COLS, "%*s}", indent, "");
                strcpy(lines[current_line + 1], temp);
                
                current_col = indent + 4;
                line_count++;
            } else {
                char temp[MAX_COLS];
                snprintf(temp, MAX_COLS, "%*s%s", indent, "", lines[current_line]);
                strcpy(lines[current_line], temp);
                current_col = indent;
            }
            
            need_redraw = true;
            break;
        case KEY_BACKSPACE:
            save_undo_state();
            if (current_col > 0) {
                memmove(&lines[current_line][current_col - 1], &lines[current_line][current_col], strlen(lines[current_line]) - current_col + 1);
                current_col--;
                need_redraw = true;
            } else if (current_line > 0) {
                current_col = strlen(lines[current_line - 1]);
                if (current_col + strlen(lines[current_line]) < MAX_COLS) {
                    strcat(lines[current_line - 1], lines[current_line]);
            
                    for (int i = current_line; i < line_count - 1; i++) {
                        strcpy(lines[i], lines[i + 1]);
                    }
                    
                    line_count--;
                    current_line--;
                    need_redraw = true;
                }
            }
            break;
        case KEY_F(7): {
            char file_dir[MAX_PATH] = {0};
            if (file_name[0] != '\0') {
                strncpy(file_dir, file_name, sizeof(file_dir) - 1);
                char *last_slash = strrchr(file_dir, '/');
                if (last_slash != NULL) {
                    *last_slash = '\0';
                    update_git_status(file_dir);
                } else {
                    getcwd(file_dir, MAX_PATH);
                    update_git_status(file_dir);
                }
            } else {
                getcwd(file_dir, MAX_PATH);
                update_git_status(file_dir);
            }
            git_status_window();
            need_redraw = true;
            }
            break;
        case KEY_F(9): console(); break;
        case 6: ctrl_f(); break;
        case 26: ctrl_z(); break;
        case 546: ctrl_left_arrow(lines[current_line]); break;
        case 561: ctrl_right_arrow(lines[current_line]); break;
        case KEY_RESIZE: apply_resize(); break;
        case 544:                   // Alt+Left - Go to start of line, provisional
            current_col = 0;
            need_redraw = true;
            break;
        case 559:                   // Alt+Right - Go to end of line, provisional
            current_col = strlen(lines[current_line]);
            need_redraw = true;
            break;
        default:
            if (c >= 32 && c <= 126) {
                insert_char(c);
                if (checks(c)) {
                    if (current_col > 0) {
                        current_col--;
                    }
                }
                need_redraw = true;
                detect_variables(lines[current_line]);
            }
            break;
    }

    getmaxyx(stdscr, rows, cols);
    int visible_width = cols - 6;
    
    if (current_col < horizontal_offset) {
        horizontal_offset = current_col;
        need_redraw = true;
    } else if (current_col >= horizontal_offset + visible_width) {
        horizontal_offset = current_col - visible_width + 1;
        need_redraw = true;
    }
    
    int screen_line = current_line;
    int start_line = 0;
    if (line_count > row - 1) {
        if (current_line > row / 2) {
            start_line = current_line - row / 2;
            screen_line = row / 2;
        }
        if (start_line + row > line_count) {
            start_line = line_count - row + 1;
            screen_line = current_line - start_line;
        }
    }
    
    move(screen_line, 6 + current_col - horizontal_offset);
    
    if (need_redraw) update_screen_content(start_line);
    
    update_status_bar();
    
    move(screen_line, 6 + current_col - horizontal_offset);
    
    curs_set(1);
    
    wnoutrefresh(stdscr);
    doupdate();
}

void init_editor() {    
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    start_color();
    if (can_change_color()) {
        init_color(8, 150, 250, 900);       // Dark blue
        init_color(9, 600, 0, 600);     // Purple
        init_color(10, 1000, 1000, 0);  // Bright yellow for functions
        init_color(11, 800, 800, 0);    // Dark yellow for parentheses
        init_color(12, 1000, 500, 0);   // Orange for strings
        init_color(13, 700, 150, 250);
    }
    
    init_pair(1, 8, COLOR_BLACK);   // Dark blue for type keywords
    init_pair(2, 9, COLOR_BLACK);   // Purple for purple keywords
    init_pair(3, 11, COLOR_BLACK);  // Bright yellow for functions
    init_pair(4, 10, COLOR_BLACK);  // Dark yellow for parentheses
    init_pair(5, COLOR_CYAN, COLOR_BLACK);  // Cyan for variables
    init_pair(6, 12, COLOR_BLACK);      // Orange for strings
    init_pair(7, COLOR_GREEN, COLOR_BLACK); // Green for comments
    init_pair(8, 13, COLOR_BLACK); // Light green for typedefs potentially
}

int main(int argc, char* argv[]) {    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage();
            exit(0);
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            exit(0);
        }
    }
    init_editor();
    load_config(&config);
    init_lines();
    display_info();
    if (open_file_browser) {
        filesystem(current_path);
    }
    args(argc, argv);

    for (int i = 0; i < MAX_UNDO; i++) {
        undo_history[i].lines = NULL;
        undo_history[i].line_count = 0;
    }

    for (int i = 0; i < line_count; i++) detect_variables(lines[i]);

    display_lines();
    update_screen_content(0);

    while (1) editor();

    cleanup_lines();
    cleanup_undo_history();
    cleanup_variables();
    endwin();
    return 0;
}