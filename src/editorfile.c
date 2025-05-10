#include "editorfile.h"

char current_path[MAX_PATH];

void create_dir(const char* current_path);
void file_save();
void create_file(const char* current_path);

void file_explorer_help() {
    int row, col;
    getmaxyx(stdscr, row, col);
    
    WINDOW *help = newwin(14, 60, row/2 - 7, col/2 - 30);
    box(help, 0, 0);
    wattron(help, A_BOLD);
    mvwprintw(help, 0, 24, " HELP ");
    wattroff(help, A_BOLD);
    
    mvwprintw(help, 2, 2, "Navigation:");
    mvwprintw(help, 3, 4, "↑/↓         Navigate through files and directories");
    mvwprintw(help, 4, 4, "Enter       Open file or enter directory");
    mvwprintw(help, 5, 4, "Backspace   Go up one directory level");
    mvwprintw(help, 6, 4, "Esc         Return to editor");
    
    mvwprintw(help, 8, 2, "Operations:");
    mvwprintw(help, 9, 4, "F1          Create new directory");
    mvwprintw(help, 10, 4, "F2          Create new file");
    mvwprintw(help, 11, 4, "F3          Open selected file");
    
    wattron(help, A_BOLD);
    mvwprintw(help, 13, 15, "Press any key to continue");
    wattroff(help, A_BOLD);
    
    wrefresh(help);
    wgetch(help);
    delwin(help);
}

char* filesystem(const char* dir_path) {
    static char selected_path[MAX_PATH];
    DIR *dir;
    struct dirent *entry;
    int selected = 0, start = 0, max_display = LINES - 6;

    init_pair(10, COLOR_WHITE, COLOR_BLUE);
    init_pair(11, COLOR_YELLOW, COLOR_BLACK);
    init_pair(12, COLOR_WHITE, COLOR_BLACK);
    init_pair(13, COLOR_BLACK, COLOR_CYAN);

    if (dir_path && *dir_path) {
        strncpy(current_path, dir_path, sizeof(current_path) - 1);
        current_path[sizeof(current_path) - 1] = '\0';
    } else if (getcwd(current_path, sizeof(current_path)) == NULL) {
        mvprintw(LINES-1, 0, "Error getting current directory");
        getch();
        return NULL;
    }
    
    dir = opendir(current_path);
    if (dir == NULL) {
        mvprintw(LINES-1, 0, "Error: Cannot open directory '%s'", current_path);
        getch();
        return NULL;
    }
    closedir(dir);

    while (1) {
        while (1) {
            clear();
            
            attron(COLOR_PAIR(10) | A_BOLD);
            for (int i = 0; i < COLS; i++) mvaddch(0, i, ' ');
            mvprintw(0, 2, "File Explorer: %s", current_path);
            mvprintw(0, COLS - 18, "Press H for Help");
            attroff(COLOR_PAIR(10) | A_BOLD);
            
            box(stdscr, 0, 0);
            
            dir = opendir(current_path);
            if (dir == NULL) {
                mvprintw(LINES-2, 2, "Error opening directory");
                getch();
                return NULL;
            }
            
            int count = 0;
            while ((entry = readdir(dir)) != NULL && count < start + max_display) {
                if (count >= start) {
                    if (count - start == selected) {
                        attron(COLOR_PAIR(13) | A_BOLD);
                    } else if (entry->d_type == DT_DIR) {
                        attron(COLOR_PAIR(11) | A_BOLD);
                    } else {
                        attron(COLOR_PAIR(12));
                    }
                    
                    mvprintw(count - start + 2, 2, "%s%s", entry->d_name, 
                            (entry->d_type == DT_DIR) ? "/" : "");
                    
                    if (count - start == selected) {
                        attroff(COLOR_PAIR(13) | A_BOLD);
                    } else if (entry->d_type == DT_DIR) {
                        attroff(COLOR_PAIR(11) | A_BOLD);
                    } else {
                        attroff(COLOR_PAIR(12));
                    }
                }
                count++;
            }
            closedir(dir);
            
            attron(COLOR_PAIR(10));
            for (int i = 0; i < COLS; i++) mvaddch(LINES-1, i, ' ');
            mvprintw(LINES-1, 2, "Up/Down: Navigate | Enter: Select | Esc: Cancel | F1: New Dir | F2: New File");
            attroff(COLOR_PAIR(10));
            
            int ch = getch();
            switch (ch) {
                case KEY_UP:
                    if (selected > 0) selected--;
                    if (selected < start) start = selected;
                    break;
                case KEY_DOWN:
                    if (selected < count - 1) selected++;
                    if (selected >= start + max_display) start++;
                    break;
                case 10:
                    dir = opendir(current_path);
                    for (int i = 0; i <= selected; i++) {
                        entry = readdir(dir);
                    }
                    closedir(dir);
                    if (entry->d_type == DT_DIR) {
                        if (strcmp(entry->d_name, "..") == 0) {
                            char *last_slash = strrchr(current_path, '/');
                            if (last_slash != current_path) {
                                *last_slash = '\0';
                            }
                        } else if (strcmp(entry->d_name, ".") != 0) {
                            strcat(current_path, "/");
                            strcat(current_path, entry->d_name);
                        }
                        selected = 0;
                        start = 0;
                    } else {
                        snprintf(selected_path, MAX_PATH, "%s/%s", current_path, entry->d_name);
                        load_file(selected_path);
                        return selected_path;
                    }
                    break;
                case KEY_BACKSPACE:
                case 127:
                    {
                        char *last_slash = strrchr(current_path, '/');
                        if (last_slash != current_path) {
                            *last_slash = '\0';
                            selected = 0;
                            start = 0;
                        }
                    }
                    break;
                case 27:
                    return NULL;
                case KEY_F(1):
                    create_dir(current_path);
                    break;
                case KEY_F(2):
                    create_file(current_path);
                    break;
                case 'h':
                case 'H':
                    file_explorer_help();
                    break;
                case KEY_F(3):
                    load_file(current_path);
                    break;
            }
        }
    }
}

void load_file(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        mvprintw(LINES-1, 0, "Error: Unable to open file %s", filepath);
        getch();
        return;
    }
    memset(lines, 0, sizeof(lines));
    line_count = 0;
    current_line = 0;
    current_col = 0;
    char buffer[MAX_COLS];
    while (fgets(buffer, MAX_COLS, file) != NULL && line_count < MAX_LINES) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            len--;
        }

        strncpy(lines[line_count], buffer, MAX_COLS - 1);
        lines[line_count][MAX_COLS - 1] = '\0';
        line_count++;
    }
    for (int i = 0; i < line_count; i++) {
        detect_variables(lines[i]);
    }
    fclose(file);
    if (line_count == 0) {
        strcpy(lines[0], "");
        line_count = 1;
    }
    strncpy(file_name, filepath, sizeof(file_name) - 1);
    file_name[sizeof(file_name) - 1] = '\0';
    mvprintw(LINES-1, 0, "File loaded successfully: %s", filepath);
}

void create_file(const char* current_path) {
    char file_name[MAX_PATH];
    char full_path[MAX_PATH];
    int i = 0;
    
    WINDOW *dialog = newwin(8, 60, LINES/2 - 4, COLS/2 - 30);
    box(dialog, 0, 0);
    wattron(dialog, A_BOLD);
    mvwprintw(dialog, 0, 20, " New File ");
    wattroff(dialog, A_BOLD);
    
    mvwprintw(dialog, 2, 2, "Enter new file name:");
    mvwprintw(dialog, 4, 2, "File name: ");
    wrefresh(dialog);
    
    while (1) {
        int c = getch();
        if (c == '\n') {
            file_name[i] = '\0';
            break;
        } else if (c == ESCAPE) {
            delwin(dialog);
            return;
        } else if (c == KEY_BACKSPACE || c == 127) {
            if (i > 0) {
                i--;
                mvwaddch(dialog, 4, 13 + i, ' ');
                wmove(dialog, 4, 13 + i);
                wrefresh(dialog);
            }
        } else if (i < MAX_PATH - 1 && c >= 32 && c <= 126) {
            file_name[i] = c;
            mvwaddch(dialog, 4, 13 + i, c);
            i++;
            wrefresh(dialog);
        }
    }
    
    if (strlen(file_name) > 0) {
        snprintf(full_path, MAX_PATH, "%s/%s", current_path, file_name);
        FILE *fp = fopen(full_path, "w");
        if (fp != NULL) {
            fclose(fp);
            mvwprintw(dialog, 6, 2, "File created successfully!");
        } else {
            mvwprintw(dialog, 6, 2, "Error: %s", strerror(errno));
        }
    } else {
        mvwprintw(dialog, 6, 2, "Error: No file name entered");
    }
    
    wrefresh(dialog);
    wgetch(dialog);
    delwin(dialog);
}

void create_dir(const char* current_path) {
    char dir_name[MAX_PATH];
    char full_path[MAX_PATH];
    int i = 0;
    
    WINDOW *dialog = newwin(8, 60, LINES/2 - 4, COLS/2 - 30);
    box(dialog, 0, 0);
    wattron(dialog, A_BOLD);
    mvwprintw(dialog, 0, 20, " New Directory ");
    wattroff(dialog, A_BOLD);
    
    mvwprintw(dialog, 2, 2, "Enter new directory name:");
    mvwprintw(dialog, 4, 2, "Directory name: ");
    wrefresh(dialog);
    
    while (1) {
        int c = getch();
        if (c == '\n') {
            dir_name[i] = '\0';
            break;
        } else if (c == ESCAPE) {
            delwin(dialog);
            return;
        } else if (c == KEY_BACKSPACE || c == 127) {
            if (i > 0) {
                i--;
                mvwaddch(dialog, 4, 17 + i, ' ');
                wmove(dialog, 4, 17 + i);
                wrefresh(dialog);
            }
        } else if (i < MAX_PATH - 1 && c >= 32 && c <= 126) {
            dir_name[i] = c;
            mvwaddch(dialog, 4, 17 + i, c);
            i++;
            wrefresh(dialog);
        }
    }
    
    if (strlen(dir_name) > 0) {
        snprintf(full_path, MAX_PATH, "%s/%s", current_path, dir_name);
        if (mkdir(full_path, 0777) == 0) {
            mvwprintw(dialog, 6, 2, "Directory created successfully!");
        } else {
            mvwprintw(dialog, 6, 2, "Error: %s", strerror(errno));
        }
    } else {
        mvwprintw(dialog, 6, 2, "Error: No directory name entered");
    }
    
    wrefresh(dialog);
    wgetch(dialog);
    delwin(dialog);
}

void file_save() {
    char temp_name[64] = {0};
    int i = 0;
    
    WINDOW *dialog = newwin(8, 60, LINES/2 - 4, COLS/2 - 30);
    box(dialog, 0, 0);
    wattron(dialog, A_BOLD);
    mvwprintw(dialog, 0, 20, " Save File ");
    wattroff(dialog, A_BOLD);
    
    mvwprintw(dialog, 2, 2, "Enter file name to save:");
    mvwprintw(dialog, 4, 2, "File name: ");
    wrefresh(dialog);
    
    while (1) {
        int c = getch();
        if (c == '\n') {
            temp_name[i] = '\0';
            break;
        } else if (c == ESCAPE) {
            delwin(dialog);
            return;
        } else if (c == KEY_BACKSPACE || c == 127) {
            if (i > 0) {
                i--;
                mvwaddch(dialog, 4, 13 + i, ' ');
                wmove(dialog, 4, 13 + i);
                wrefresh(dialog);
            }
        } else if (i < 63 && c >= 32 && c <= 126) {
            temp_name[i] = c;
            mvwaddch(dialog, 4, 13 + i, c);
            i++;
            wrefresh(dialog);
        }
    }
    
    if (strlen(temp_name) > 0) {
        transcribe_to_text();
        strcpy(file_name, temp_name);
        FILE *fp = fopen(file_name, "w");
        if (fp == NULL) {
            mvwprintw(dialog, 6, 2, "Error: Unable to open file for writing");
        } else {
            fputs(text, fp);
            fclose(fp);
            mvwprintw(dialog, 6, 2, "File saved successfully!");
        }
    } else {
        mvwprintw(dialog, 6, 2, "Error: No file name entered");
    }
    
    wrefresh(dialog);
    wgetch(dialog);
    delwin(dialog);
}

void save_file() {
    if (file_name[0] == '\0') {
        file_save();
        return;
    }
    
    FILE *fp = fopen(file_name, "w");
    if (fp == NULL) {
        WINDOW *dialog = newwin(6, 60, LINES/2 - 3, COLS/2 - 30);
        box(dialog, 0, 0);
        wattron(dialog, A_BOLD);
        mvwprintw(dialog, 0, 24, " Error ");
        wattroff(dialog, A_BOLD);
        
        mvwprintw(dialog, 2, 2, "Unable to open file for writing: %s", file_name);
        mvwprintw(dialog, 4, 20, "Press any key to continue");
        
        wrefresh(dialog);
        wgetch(dialog);
        delwin(dialog);
        return;
    }
    
    for (int i = 0; i < line_count; i++) {
        fputs(lines[i], fp);
        if (i < line_count - 1 || lines[i][strlen(lines[i]) - 1] != '\n') {
            fputc('\n', fp);
        }
    }
    fclose(fp);
    
    WINDOW *dialog = newwin(6, 60, LINES/2 - 3, COLS/2 - 30);
    box(dialog, 0, 0);
    wattron(dialog, A_BOLD);
    mvwprintw(dialog, 0, 20, " File Saved ");
    wattroff(dialog, A_BOLD);
    
    mvwprintw(dialog, 2, 2, "File saved successfully: %s", file_name);
    mvwprintw(dialog, 4, 20, "Press any key to continue");
    
    wrefresh(dialog);
    wgetch(dialog);
    delwin(dialog);
}