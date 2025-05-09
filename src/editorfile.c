#include "editorfile.h"

char current_path[MAX_PATH];

void create_dir(const char* current_path);
void file_save();
void create_file(const char* current_path);

void file_explorer_help() {
    int row, col;
    getmaxyx(stdscr, row, col);
    row = row / 2;
    col = col / 2;
    int start = col - strlen("Press:   F1   for instructions about filesystem") / 2;
    mvprintw(row - 5, col - strlen("Info") / 2, "Info");
    mvprintw(row -3, start, "Use:\t\tArrow keys\tto navigate");
    mvprintw(row -2, start, "Press:\tEnter\t\tto select/enter");
    mvprintw(row -1, start, "Press:\tBackspace\tto go up");
    mvprintw(row, start, "Press:\tEsc\t\tto cancel");
    mvprintw(row + 1, start, "Press:\tF1\t\tto create a new directory");
    mvprintw(row + 2, start, "Press:\tF2\t\tto create a new file");
    mvprintw(row + 3, start, "Press:\tF3\t\tto load the selected file");
    mvprintw(row + 5, col - strlen("Press any key to continue") / 2, "Press any key to continue...");
    getch();
}

char* filesystem() {
    static char selected_path[MAX_PATH];
    DIR *dir;
    struct dirent *entry;
    int selected = 0, start = 0, max_display = LINES - 4;
    if (getcwd(current_path, sizeof(current_path)) == NULL) {
        mvprintw(LINES-1, 0, "Error getting current directory");
        getch();
        return NULL;
    }
    while (1) {
        clear();
        mvprintw(0, 0, "Current directory: %s\tPress H for help", current_path);
        mvprintw(1, 0, " ");
        dir = opendir(current_path);
        if (dir == NULL) {
            mvprintw(LINES-1, 0, "Error opening directory");
            getch();
            return NULL;
        }
        int count = 0;
        while ((entry = readdir(dir)) != NULL && count < start + max_display) {
            if (count >= start) {
                if (count - start == selected)
                    attron(A_REVERSE);
                mvprintw(count - start + 2, 0, "%s%s", entry->d_name, 
                         (entry->d_type == DT_DIR) ? "/" : "");
                attroff(A_REVERSE);
            }
            count++;
        }
        closedir(dir);
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
    clear();
    mvprintw(0, 0, "Enter new file name (press Enter to confirm, Esc to cancel):");
    mvprintw(2, 0, "File name: ");
    refresh();
    while (1) {
        int c = getch();
        if (c == '\n') {
            file_name[i] = '\0';
            break;
        } else if (c == ESCAPE) {
            return;
        } else if (c == KEY_BACKSPACE || c == 127) {
            if (i > 0) {
                i--;
                mvaddch(2, 11 + i, ' ');
                move(2, 11 + i);
            }
        } else if (i < MAX_PATH - 1 && c >= 32 && c <= 126) {
            file_name[i] = c;
            mvaddch(2, 11 + i, c);
            i++;
        }
        refresh();
    }
    if (strlen(file_name) > 0) {
        snprintf(full_path, MAX_PATH, "%s/%s", current_path, file_name);
        FILE *fp = fopen(full_path, "w");
        if (fp != NULL) {
            fclose(fp);
            mvprintw(4, 0, "Empty file created successfully: %s", full_path);
        } else {
            mvprintw(4, 0, "Error: Unable to create file. %s", strerror(errno));
        }
    } else {
        mvprintw(4, 0, "Error: No file name entered.");
    }
    mvprintw(6, 0, "Press any key to continue...");
    refresh();
    getch();
}

void create_dir(const char* current_path) {
    char dir_name[MAX_PATH];
    char full_path[MAX_PATH];
    int i = 0;
    clear();
    mvprintw(0, 0, "Enter new directory name (press Enter to confirm, Esc to cancel):");
    mvprintw(2, 0, "Directory name: ");
    refresh();
    while (1) {
        int c = getch();
        if (c == '\n') {
            dir_name[i] = '\0';
            break;
        } else if (c == ESCAPE) {
            return;
        } else if (c == KEY_BACKSPACE || c == 127) {
            if (i > 0) {
                i--;
                mvaddch(2, 15 + i, ' ');
                move(2, 15 + i);
            }
        } else if (i < MAX_PATH - 1 && c >= 32 && c <= 126) {
            dir_name[i] = c;
            mvaddch(2, 15 + i, c);
            i++;
        }
        refresh();
    }
    if (strlen(dir_name) > 0) {
        snprintf(full_path, MAX_PATH, "%s/%s", current_path, dir_name);
        if (mkdir(full_path, 0777) == 0) {
            mvprintw(4, 0, "Directory created successfully: %s", full_path);
        } else {
            mvprintw(4, 0, "Error: Unable to create directory. %s", strerror(errno));
        }
    } else {
        mvprintw(4, 0, "Error: No directory name entered.");
    }
    mvprintw(6, 0, "Press any key to continue...");
    refresh();
    getch();
}

void file_save() {
    clear();
    mvprintw(0, 0, "Enter file name to save (press Enter to confirm, Esc to cancel):");
    mvprintw(2, 0, "File name: ");
    refresh();
    int i = 0;
    int name_start_col = 11;
    while (1) {
        int c = getch();
        if (c == '\n') {
            file_name[i] = '\0';
            break;
        } else if (c == ESCAPE) {
            return;
        } else if (c == KEY_BACKSPACE || c == 127) {
            if (i > 0) {
                i--;
                mvaddch(2, name_start_col + i, ' ');
                move(2, name_start_col + i);
            }
        } else if (i < 63 && c >= 32 && c <= 126) {
            file_name[i] = c;
            mvaddch(2, name_start_col + i, c);
            i++;
        }
        refresh();
    }
    if (strlen(file_name) > 0) {
        transcribe_to_text();
        FILE *fp = fopen(file_name, "w");
        if (fp == NULL) {
            mvprintw(4, 0, "Error: Unable to open file for writing.");
            getch();
            return;
        }
        fputs(text, fp);
        fclose(fp);
        mvprintw(4, 0, "File saved successfully as: %s", file_name);
    } else {
        mvprintw(4, 0, "Error: No file name entered.");
    }
    mvprintw(6, 0, "Press any key to continue...");
    refresh();
    getch();
}

void save_file() {
    if (file_name[0] == '\0') {
        file_save();
        return;
    }
    FILE *fp = fopen(file_name, "w");
    if (fp == NULL) {
        clear();
        mvprintw(0, 0, "Error: Unable to open file for writing: %s", file_name);
        mvprintw(2, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    for (int i = 0; i < line_count; i++) {
        fputs(lines[i], fp);
        if (i < line_count - 1 || lines[i][strlen(lines[i]) - 1] != '\n') {
            fputc('\n', fp);
        }
    }
    fclose(fp);
    clear();
    mvprintw(0, 0, "File saved successfully: %s", file_name);
    mvprintw(2, 0, "Press any key to continue...");
    refresh();
    getch();
}