#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "sceconfig.h"
#include "library.h"
#include "macros.h"
#include "editorfile.h"

#define MAX_LINE_LENGTH 256
#define CONFIG_FILE_NAME ".sceconfig"
#define MAX_CONFIG_ITEMS 14
#define MAX_VALUE_LENGTH 32

/*
*   +-----------------------+
*   |TODO: implement colors |
*   +-----------------------+
*/

extern EditorConfig config;
extern int current_line;
extern int current_col;
char original_value[MAX_VALUE_LENGTH] = "";
int edit_started = 0;

static void trim_string(char *str);
static bool parse_boolean(const char *value);
static int parse_integer(const char *value, int default_value);
void save_config(const EditorConfig *config);
void load_config(EditorConfig *config);

typedef struct ConfigOption {
    char name[32];
    char description[64];
    char value[MAX_VALUE_LENGTH];
    int type;   //0 = int, 1 = bool, 2 = string
} ConfigOption;

static char* get_config_file_path() {
    static char config_path[MAX_PATH];
    char* home_dir = getenv("HOME");
    
    if (home_dir) {
        snprintf(config_path, MAX_PATH, "%s/.sceconfig/%s", home_dir, CONFIG_FILE_NAME);
    } else {
        snprintf(config_path, MAX_PATH, "./%s", CONFIG_FILE_NAME);
    }
    
    return config_path;
}

char* config_file_browser(const char* start_path) {
    static char selected_path[MAX_PATH];
    static char current_path[MAX_PATH];
    DIR *dir;
    struct dirent *entry;
    int selected = 0, start = 0, max_display = LINES - 6;
    
    init_pair(30, COLOR_WHITE, COLOR_BLUE);
    init_pair(31, COLOR_YELLOW, COLOR_BLACK);
    init_pair(32, COLOR_WHITE, COLOR_BLACK);
    init_pair(33, COLOR_BLACK, COLOR_CYAN);
    init_pair(34, COLOR_WHITE, COLOR_BLUE);

    if (!start_path || strlen(start_path) == 0) {
        if (getcwd(current_path, sizeof(current_path)) == NULL) {
            strcpy(current_path, ".");
        }
    } else {
        strncpy(current_path, start_path, sizeof(current_path) - 1);
        current_path[sizeof(current_path) - 1] = '\0';
    }
    
    dir = opendir(current_path);
    if (dir == NULL) {
        if (getcwd(current_path, sizeof(current_path)) == NULL) {
            strcpy(current_path, ".");
        }
        dir = opendir(current_path);
        if (dir == NULL) {
            return NULL;
        }
    }
    closedir(dir);

    def_prog_mode();
    
    while (1) {
        clear();
        
        attron(COLOR_PAIR(30) | A_BOLD);
        for (int i = 0; i < COLS; i++) mvaddch(0, i, ' ');
        mvprintw(0, 2, "Select Path: %s", current_path);
        attroff(COLOR_PAIR(30) | A_BOLD);
        
        dir = opendir(current_path);
        if (dir == NULL) {
            mvprintw(LINES-2, 2, "Error opening directory");
            refresh();
            getch();
            return NULL;
        }
        
        int count = 0;
        while ((entry = readdir(dir)) != NULL && count < start + max_display) {
            if (count >= start) {
                if (count - start == selected) {
                    attron(COLOR_PAIR(33) | A_BOLD);
                } else if (entry->d_type == DT_DIR) {
                    attron(COLOR_PAIR(31) | A_BOLD);
                } else {
                    attron(COLOR_PAIR(32));
                }
                
                mvprintw(count - start + 2, 2, "%s%s", entry->d_name, 
                        (entry->d_type == DT_DIR) ? "/" : "");
                
                if (count - start == selected) {
                    attroff(COLOR_PAIR(33) | A_BOLD);
                } else if (entry->d_type == DT_DIR) {
                    attroff(COLOR_PAIR(31) | A_BOLD);
                } else {
                    attroff(COLOR_PAIR(32));
                }
            }
            count++;
        }
        closedir(dir);
        
        attron(COLOR_PAIR(34));
        for (int i = 0; i < COLS; i++) mvaddch(LINES-1, i, ' ');
        mvprintw(LINES-1, 2, "Up/Down: Navigate | Enter: Select Dir | F10: Use Current Dir | Esc: Cancel");
        attroff(COLOR_PAIR(34));
        
        refresh();
        
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
            case KEY_F(10):
                strncpy(selected_path, current_path, sizeof(selected_path) - 1);
                selected_path[sizeof(selected_path) - 1] = '\0';
                reset_prog_mode();
                return selected_path;
            case 27:
                reset_prog_mode();
                return NULL;
        }
    }
}

void config_editor() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    def_prog_mode();
    
    EditorConfig current_config = {0};
    
    current_config.tab_size = config.tab_size;
    current_config.parenthesis_autocomplete = config.parenthesis_autocomplete;
    current_config.quotations_autocomplete = config.quotations_autocomplete;
    current_config.autosave = config.autosave;
    current_config.color_0 = config.color_0;
    current_config.color_1 = config.color_1;
    current_config.color_2 = config.color_2;
    current_config.color_3 = config.color_3;
    current_config.color_4 = config.color_4;
    current_config.color_5 = config.color_5;
    current_config.color_6 = config.color_6;
    current_config.color_7 = config.color_7;
    current_config.color_8 = config.color_8;
    
    if (config.default_path != NULL) {
        current_config.default_path = strdup(config.default_path);
    } else {
        current_config.default_path = strdup("");
    }
    
    ConfigOption options[MAX_CONFIG_ITEMS] = {
        {"tab_size", "Number of spaces per tab", "", 0},
        {"parenthesis_autocomplete", "Auto-complete parentheses (0=off, 1=on)", "", 1},
        {"quotations_autocomplete", "Auto-complete quotes (0=off, 1=on)", "", 1},
        {"autosave", "File autosave (0=off, 1=on)", "", 1},
        {"color_0", "Comments color (0-255, 0=default)", "", 0},
        {"color_1", "Types color (0-255, 0=default)", "", 0},
        {"color_2", "Control flow color (0-255, 0=default)", "", 0},
        {"color_3", "Variables color (0-255, 0=default)", "", 0},
        {"color_4", "Functions color (0-255, 0=default)", "", 0},
        {"color_5", "Numbers color (0-255, 0=default)", "", 0},
        {"color_6", "Parentheses color (0-255, 0=default)", "", 0},
        {"color_7", "Strings color (0-255, 0=default)", "", 0},
        {"color_8", "Typedef color (0-255, 0=default)", "", 0},
        {"path", "Default file path", "", 2}
    };
    
    sprintf(options[0].value, "%d", current_config.tab_size);
    sprintf(options[1].value, "%d", current_config.parenthesis_autocomplete ? 1 : 0);
    sprintf(options[2].value, "%d", current_config.quotations_autocomplete ? 1 : 0);
    sprintf(options[3].value, "%d", current_config.autosave);
    sprintf(options[4].value, "%d", current_config.color_0);
    sprintf(options[5].value, "%d", current_config.color_1);
    sprintf(options[6].value, "%d", current_config.color_2);
    sprintf(options[7].value, "%d", current_config.color_3);
    sprintf(options[8].value, "%d", current_config.color_4);
    sprintf(options[9].value, "%d", current_config.color_5);
    sprintf(options[10].value, "%d", current_config.color_6);
    sprintf(options[11].value, "%d", current_config.color_7);
    sprintf(options[12].value, "%d", current_config.color_8);
    strncpy(options[13].value, current_config.default_path ? current_config.default_path : "", MAX_VALUE_LENGTH - 1);
    
    int current_item = 0;
    int editing = 0;
    
    init_pair(20, COLOR_BLACK, COLOR_WHITE);
    init_pair(21, COLOR_YELLOW, COLOR_BLUE);
    init_pair(22, COLOR_WHITE, COLOR_BLUE);
    
    int ch;
    int done = 0;
    
    while (!done) {
        clear();
        
        attron(COLOR_PAIR(21));
        for (int i = 0; i < cols; i++) {
            mvaddch(0, i, ' ');
        }
        mvprintw(0, (cols - 20) / 2, "SCE Configuration");
        attroff(COLOR_PAIR(21));
        
        for (int i = 0; i < MAX_CONFIG_ITEMS - 1; i++) {
            if (i == current_item) {
                attron(COLOR_PAIR(20));
            }
            
            mvprintw(i + 2, 2, "%-25s %-40s [%s]", 
                    options[i].name, options[i].description, options[i].value);
            
            if (i == current_item) {
                attroff(COLOR_PAIR(20));
            }
        }
        
        attron(COLOR_PAIR(22));
        for (int i = 0; i < cols; i++) {
            mvaddch(rows - 1, i, ' ');
        }
        mvprintw(rows - 1, 2, "Up/Down: Navigate | Enter: Edit | F10: Save | Esc: Cancel");
        attroff(COLOR_PAIR(22));
        
        if (editing) {
            move(current_item + 2, 70);
        }
        
        refresh();
        
        ch = getch();
        
        if (editing) {
            mvprintw(current_item + 2, 69, "[%*s]", MAX_VALUE_LENGTH - 1, "");
    
            move(current_item + 2, 70);
            printw("%s", options[current_item].value);

            if (!edit_started) {
                strncpy(original_value, options[current_item].value, MAX_VALUE_LENGTH - 1);
                original_value[MAX_VALUE_LENGTH - 1] = '\0';
                
                options[current_item].value[0] = '\0';
                edit_started = 1;
            }

            switch (ch) {
                case '\n':
                case KEY_ENTER:
                    if (strlen(options[current_item].value) == 0) {
                        strncpy(options[current_item].value, original_value, MAX_VALUE_LENGTH - 1);
                        options[current_item].value[MAX_VALUE_LENGTH - 1] = '\0';
                    }
                    editing = 0;
                    edit_started = 0;
                    break;
                case 27:
                    strncpy(options[current_item].value, original_value, MAX_VALUE_LENGTH - 1);
                    options[current_item].value[MAX_VALUE_LENGTH - 1] = '\0';
                    editing = 0;
                    edit_started = 0;
                    break;
                case KEY_BACKSPACE:
                case 127:
                case '\b':
                    if (strlen(options[current_item].value) > 0) {
                        options[current_item].value[strlen(options[current_item].value) - 1] = '\0';
                    }
                    break;
                default:
                    if ((options[current_item].type == 0 && (isdigit(ch) || ch == '-')) ||
                        (options[current_item].type == 1 && (ch == '0' || ch == '1')) ||
                        (options[current_item].type == 2 && ch >= 32 && ch < 127)) {
                        
                        if (strlen(options[current_item].value) < MAX_VALUE_LENGTH - 1) {
                            size_t len = strlen(options[current_item].value);
                            options[current_item].value[len] = ch;
                            options[current_item].value[len + 1] = '\0';
                        }
                    }
                    break;
            }
        } else {
            switch (ch) {
                case KEY_UP:
                    current_item = (current_item - 1 + MAX_CONFIG_ITEMS - 1) % (MAX_CONFIG_ITEMS - 1);
                    break;
                case KEY_DOWN:
                    current_item = (current_item + 1) % (MAX_CONFIG_ITEMS - 1);
                    break;
                case '\n':
                case KEY_ENTER:
                if (current_item == 13) {
                    char* previous_path = NULL;
                    
                    if (current_config.default_path && strlen(current_config.default_path) > 0) {
                        previous_path = current_config.default_path;
                    }
                    
                    char* result_path = config_file_browser(previous_path);
                    
                    if (result_path != NULL) {
                        strncpy(options[13].value, result_path, MAX_VALUE_LENGTH - 1);
                        options[13].value[MAX_VALUE_LENGTH - 1] = '\0';
                        
                        if (current_config.default_path) {
                            free(current_config.default_path);
                        }
                        current_config.default_path = strdup(result_path);
                    }
                } else {
                    editing = 1;
                }
                    break;
                case KEY_F(10):
                    current_config.tab_size = atoi(options[0].value);
                    current_config.parenthesis_autocomplete = atoi(options[1].value) != 0;
                    current_config.quotations_autocomplete = atoi(options[2].value) != 0;
                    current_config.autosave = atoi(options[3].value);
                    current_config.color_0 = atoi(options[4].value);
                    current_config.color_1 = atoi(options[5].value);
                    current_config.color_2 = atoi(options[6].value);
                    current_config.color_3 = atoi(options[7].value);
                    current_config.color_4 = atoi(options[8].value);
                    current_config.color_5 = atoi(options[9].value);
                    current_config.color_6 = atoi(options[10].value);
                    current_config.color_7 = atoi(options[11].value);
                    current_config.color_8 = atoi(options[12].value);
                    
                    if (current_config.default_path) {
                        free(current_config.default_path);
                    }
                    current_config.default_path = strdup(options[13].value);
                    
                    save_config(&current_config);
                    
                    config.tab_size = current_config.tab_size;
                    config.parenthesis_autocomplete = current_config.parenthesis_autocomplete;
                    config.quotations_autocomplete = current_config.quotations_autocomplete;
                    config.autosave = current_config.autosave;
                    config.color_0 = current_config.color_0;
                    config.color_1 = current_config.color_1;
                    config.color_2 = current_config.color_2;
                    config.color_3 = current_config.color_3;
                    config.color_4 = current_config.color_4;
                    config.color_5 = current_config.color_5;
                    config.color_6 = current_config.color_6;
                    config.color_7 = current_config.color_7;
                    config.color_8 = current_config.color_8;
                    
                    if (config.default_path) {
                        free(config.default_path);
                    }
                    config.default_path = strdup(current_config.default_path);

                    if (config.color_0 > 0) {
                        int red = ((config.color_0 >> 5) & 0x7) * 1000 / 7;
                        int green = ((config.color_0 >> 2) & 0x7) * 1000 / 7;
                        int blue = (config.color_0 & 0x3) * 1000 / 3;
                        init_color(COLOR_GREEN, red, green, blue);             // Comments
                    }
                    
                    if (config.color_1 >    0) {
                        int red = ((config.color_1 >> 5) & 0x7) * 1000 / 7;
                        int green = ((config.color_1 >> 2) & 0x7) * 1000 / 7;
                        int blue = (config.color_1 & 0x3) * 1000 / 3;
                        init_color(8, red, green, blue);                       // Types
                    }
                    
                    if (config.color_2 > 0) {
                        int red = ((config.color_2 >> 5) & 0x7) * 1000 / 7;
                        int green = ((config.color_2 >> 2) & 0x7) * 1000 / 7;
                        int blue = (config.color_2 & 0x3) * 1000 / 3;
                        init_color(9, red, green, blue);                       // Control flow
                    }

                    if (config.color_3 > 0) {
                        int red = ((config.color_3 >> 5) & 0x7) * 1000 / 7;
                        int green = ((config.color_3 >> 2) & 0x7) * 1000 / 7;
                        int blue = (config.color_3 & 0x3) * 1000 / 3;
                        init_color(COLOR_CYAN, red, green, blue);             // Variables
                    }
                    
                    if (config.color_4 > 0) {
                        int red = ((config.color_4 >> 5) & 0x7) * 1000 / 7;
                        int green = ((config.color_4 >> 2) & 0x7) * 1000 / 7;
                        int blue = (config.color_4 & 0x3) * 1000 / 3;
                        init_color(11, red, green, blue);                      // Functions
                    }
                    
                    if (config.color_6 > 0) {
                        int red = ((config.color_6 >> 5) & 0x7) * 1000 / 7;
                        int green = ((config.color_6 >> 2) & 0x7) * 1000 / 7;
                        int blue = (config.color_6 & 0x3) * 1000 / 3;
                        init_color(10, red, green, blue);                      // Parentheses
                    }
                    
                    if (config.color_7 > 0) {
                        int red = ((config.color_7 >> 5) & 0x7) * 1000 / 7;
                        int green = ((config.color_7 >> 2) & 0x7) * 1000 / 7;
                        int blue = (config.color_7 & 0x3) * 1000 / 3;
                        init_color(12, red, green, blue);                      // Strings
                    }
                    
                    if (config.color_8 > 0) {
                        int red = ((config.color_8 >> 5) & 0x7) * 1000 / 7;
                        int green = ((config.color_8 >> 2) & 0x7) * 1000 / 7;
                        int blue = (config.color_8 & 0x3) * 1000 / 3;
                        init_color(13, red, green, blue);                      // Typedefs
                    }
                    
                    done = 1;
                    break;
                case 27:
                    done = 1;
                    break;
            }
        }
    }
    
    if (current_config.default_path) {
        free(current_config.default_path);
    }

    reset_prog_mode();
    clear();
    refresh();
    update_screen_content(current_line);
}

void load_config(EditorConfig *config) {
    //Set default configuration values
    config->tab_size = 4;
    config->parenthesis_autocomplete = true;
    config->quotations_autocomplete = true;
    config->autosave = true;
    
    config->color_0 = 0;   //Comments
    config->color_1 = 0;   //Types
    config->color_2 = 0;   // Control flow
    config->color_3 = 0;   //Variables
    config->color_4 = 0;   //Function calls
    config->color_5 = 0;   //Numbers
    config->color_6 = 0;   //Parentheses
    config->color_7 = 0;   //Strings
    config->color_8 = 0;   //Typedef
    
    FILE *config_file = fopen(get_config_file_path(), "r");
    if (!config_file) return;
    
    char line[MAX_LINE_LENGTH];
    
    while (fgets(line, MAX_LINE_LENGTH, config_file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        
        char *equals = strchr(line, '=');
        if (!equals) {
            continue;
        }
        
        *equals = '\0';
        char *key = line;
        char *value = equals + 1;
        
        trim_string(key);
        trim_string(value);
        
        if (strcmp(key, "tab_size") == 0) {
            config->tab_size = parse_integer(value, 4);
        } else if (strcmp(key, "autocomplete") == 0) {
            int autocomplete = parse_integer(value, 1);
            config->parenthesis_autocomplete = (autocomplete > 0);
            config->quotations_autocomplete = (autocomplete > 0);
        } else if (strcmp(key, "parenthesis_autocomplete") == 0) {
            config->parenthesis_autocomplete = parse_boolean(value);
        } else if (strcmp(key, "quotations_autocomplete") == 0) {
            config->quotations_autocomplete = parse_boolean(value);
        } else if (strcmp(key, "autosave") == 0) {
            config->autosave = parse_boolean(value);
        } else if (strcmp(key, "color_0") == 0) {
            config->color_0 = parse_integer(value, 0);
        } else if (strcmp(key, "color_1") == 0) {
            config->color_1 = parse_integer(value, 0);
        } else if (strcmp(key, "color_2") == 0) {
            config->color_2 = parse_integer(value, 0);
        } else if (strcmp(key, "color_3") == 0) {
            config->color_3 = parse_integer(value, 0);
        } else if (strcmp(key, "color_4") == 0) {
            config->color_4 = parse_integer(value, 0);
        } else if (strcmp(key, "color_5") == 0) {
            config->color_5 = parse_integer(value, 0);
        } else if (strcmp(key, "color_6") == 0) {
            config->color_6 = parse_integer(value, 0);
        } else if (strcmp(key, "color_7") == 0) {
            config->color_7 = parse_integer(value, 0);
        } else if (strcmp(key, "color_8") == 0) {
            config->color_8 = parse_integer(value, 0);
        } else if (strcmp(key, "path") == 0) {
            if (value && strlen(value) > 0) {
                if (config->default_path) {
                    free(config->default_path);
                }
                config->default_path = strdup(value);
            }
        }
    }
    
    fclose(config_file);
}

void save_config(const EditorConfig *config) {
    FILE *config_file = fopen(get_config_file_path(), "w");
    if (!config_file) {
        return;
    }
    
    fprintf(config_file, "# size for tabs (spaces)\n");
    fprintf(config_file, "tab_size=%d\n\n", config->tab_size);
    
    fprintf(config_file, "# specific autocompletion settings\n");
    fprintf(config_file, "parenthesis_autocomplete=%d\n", config->parenthesis_autocomplete ? 1 : 0);
    fprintf(config_file, "quotations_autocomplete=%d\n\n", config->quotations_autocomplete ? 1 : 0);
    fprintf(config_file, "# automatic file saving\n");
    fprintf(config_file, "autosave=%d\n\n", config->autosave ? 1 : 0);
    
    fprintf(config_file, "# color sets are based on vs code\n");
    fprintf(config_file, "# color 0: comments;          vs code dark green\n\n");
    fprintf(config_file, "# color 1: types;             vs code blue\n");
    fprintf(config_file, "# color 2: control flow;      vs code purple\n");
    fprintf(config_file, "# color 3: variables;         vs code cyan\n");
    fprintf(config_file, "# color 4: function calls;    vs code light yellow\n");
    fprintf(config_file, "# color 5: numbers;           vs code light green\n");
    fprintf(config_file, "# color 6: parentheses;       vs code variable\n");
    fprintf(config_file, "# color 7: strings;           vs code orange\n");
    fprintf(config_file, "# color 8: type definitions;       vs code green\n");
    
    fprintf(config_file, "color_0=%d\n", config->color_0);
    fprintf(config_file, "color_1=%d\n", config->color_1);
    fprintf(config_file, "color_2=%d\n", config->color_2);
    fprintf(config_file, "color_3=%d\n", config->color_3);
    fprintf(config_file, "color_4=%d\n", config->color_4);
    fprintf(config_file, "color_5=%d\n", config->color_5);
    fprintf(config_file, "color_6=%d\n", config->color_6);
    fprintf(config_file, "color_7=%d\n", config->color_7);
    fprintf(config_file, "color_8=%d\n", config->color_8);
    
    fprintf(config_file, "# default path for files\n");
    fprintf(config_file, "path=%s\n", config->default_path ? config->default_path : "");
    
    fclose(config_file);
}

static void trim_string(char *str) {
    if (!str) return;
    
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return;
    
    char *end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    *(end+1) = 0;
}

static bool parse_boolean(const char *value) {
    if (!value) return false;
    
    if (strcasecmp(value, "true") == 0 ||
        strcasecmp(value, "yes") == 0 ||
        strcasecmp(value, "1") == 0 ||
        strcasecmp(value, "on") == 0) {
        return true;
    }
    
    return false;
}

static int parse_integer(const char *value, int default_value) {
    if (!value || value[0] == '\0') return default_value;
    
    char *end;
    long result = strtol(value, &end, 10);
    
    if (*end != '\0') {
        return default_value;
    }
    
    return (int)result;
}