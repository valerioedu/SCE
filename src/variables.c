#include "variables.h"
#include "colors.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

char** variables = NULL;
size_t variables_capacity = 0;
size_t variables_count = 0;

bool* variables_found = NULL;
bool scan_in_progress = false;

char** typedefs = NULL;
size_t typedefs_capacity = 0;
size_t typedefs_count = 0;
int typedef_waiting = 0;
static char pending_alias[128] = "";
static char pending_tag[128] = "";
static int struct_depth = 0;

void cleanup_typedefs() {
    if (!typedefs) return;

    for (size_t i = 0; i < typedefs_count; i++) {
        free(typedefs[i]);
    }

    free(typedefs);
    typedefs = NULL;
    typedefs_count = typedefs_capacity = 0;
}

void begin_variable_scan() {
    if (variables) {
        for (size_t i = 0; i < variables_count; i++) {
            free(variables[i]);
        }
        variables_count = 0;
    }
    
    cleanup_typedefs();
    typedef_waiting = 0;
    pending_alias[0] = '\0';
    pending_tag[0] = '\0';
    struct_depth = 0;
    if (variables_found) free(variables_found);
    variables_found = calloc(variables_capacity, sizeof(bool));
    scan_in_progress = true;
}

void finish_variable_scan() {
    if (!scan_in_progress || !variables_found) return;

    size_t i = 0;
    while (i < variables_count) {
        if (!variables_found[i]) {
            free(variables[i]);

            for (size_t j = i; j < variables_count - 1; j++) {
                variables[j] = variables[j + 1];
                variables_found[j] = variables_found[j + 1];
            }
            variables_count--;
        } else {
            i++;
        }
    }

    free(variables_found);
    variables_found = NULL;
    scan_in_progress = false;
}

void save_variables(char* var) {
    if (variables == NULL) {
        variables_capacity = 16;
        variables = malloc(variables_capacity * sizeof(char*));
        if (variables == NULL) return;
        memset(variables, 0, variables_capacity * sizeof(char*));
    }

    if (!var || !(isalpha((unsigned char)var[0]) || var[0] == '_')) return;

    if (variables_count >= variables_capacity) {
        size_t new_capacity = variables_capacity * 2;
        char** new_vars = realloc(variables, new_capacity * sizeof(char*));
        if (new_vars) {
            variables = new_vars;
            variables_capacity = new_capacity;
        } else return;
    }

    variables[variables_count++] = strdup(var);
}

static void save_typedef(const char* name) {
    if (!name || !*name) return;

    if (!typedefs) {
        typedefs_capacity = 16;
        typedefs = malloc(typedefs_capacity * sizeof(char*));
        if (!typedefs) return;
        memset(typedefs, 0, typedefs_capacity * sizeof(char*));
    }

    for (size_t i = 0; i < typedefs_count; i++) {
        if (strcmp(typedefs[i], name) == 0) return;
    }

    if (typedefs_count >= typedefs_capacity) {
        size_t new_cap = typedefs_capacity * 2;
        char** tmp = realloc(typedefs, new_cap * sizeof(char*));
        if (!tmp) return;
        typedefs = tmp;
        typedefs_capacity = new_cap;
    }

    typedefs[typedefs_count++] = strdup(name);
}

void detect_variables(char* line) {
    if (inside_multiline_comment) return;
    
    char temp[MAX_COLS];
    char* working_line = line;

    char* single_comment = strstr(line, "//");
    if (single_comment) {
        int comment_pos = single_comment - line;
        strncpy(temp, line, comment_pos);
        temp[comment_pos] = '\0';
        working_line = temp;
    }

    char* comment = strstr(line, "/*");
    if (comment) {
        int comment_2p = comment - line;
        strncpy(temp, line, comment_2p);
        temp[comment_2p] = '\0';
        working_line = temp;
    }

    char* line_ptr = working_line;
    while (*line_ptr && isspace((unsigned char)*line_ptr)) {
        line_ptr++;
    }

    if (strncmp(line_ptr, "typedef", 7) == 0 && isspace((unsigned char)line_ptr[7])) {
        char* semicolon = strchr(line_ptr, ';');
        if (semicolon) {
            char* end = semicolon - 1;
            while (end > line_ptr && isspace((unsigned char)*end)) end--;
            char* start = end;
            while (start > line_ptr && (isalnum((unsigned char)*(start-1)) || *(start-1) == '_')) start--;
            size_t len = end - start + 1;

            if (len > 0 && len < sizeof(pending_alias)) {
                strncpy(pending_alias, start, len);
                pending_alias[len] = '\0';
                save_typedef(pending_alias);
            }
        } else {
            typedef_waiting = 1;
            pending_tag[0] = '\0';
            char* p = line_ptr + 7;
            while (*p && isspace((unsigned char)*p)) p++;

            if ((strncmp(p, "struct", 6) == 0 && isspace((unsigned char)p[6])) || (strncmp(p, "union", 5) == 0 && isspace((unsigned char)p[5])) || (strncmp(p, "enum", 4) == 0 && isspace((unsigned char)p[4]))) {
                int kw_len = (p[0]=='s'?6:(p[0]=='u'?5:4));
                p += kw_len;
                while (*p && isspace((unsigned char)*p)) p++;
                char* tag_start = p;
                while (*p && (isalnum((unsigned char)*p) || *p == '_')) p++;
                size_t len = p - tag_start;

                if (len > 0 && len < sizeof(pending_tag)) {
                    strncpy(pending_tag, tag_start, len);
                    pending_tag[len] = '\0';
                }
            }
            struct_depth = 0;

            for (char* scan = line_ptr; *scan; scan++) if (*scan == '{') struct_depth++;
        }
        return;
    }
    if (typedef_waiting) {
        char* p = line_ptr;
        while (*p) {
            if (*p == '{') struct_depth++;
            else if (*p == '}') {
                if (struct_depth > 0) struct_depth--;
                if (struct_depth == 0) {
                    char* alias_start = p + 1;
                    while (*alias_start && isspace((unsigned char)*alias_start)) alias_start++;
                    char* alias_end = alias_start;
                    while (*alias_end && (isalnum((unsigned char)*alias_end) || *alias_end == '_')) alias_end++;
                    size_t len = alias_end - alias_start;
                    if (len > 0 && *alias_end == ';') {
                        if (len < sizeof(pending_alias)) {
                            strncpy(pending_alias, alias_start, len);
                            pending_alias[len] = '\0';
                            save_typedef(pending_alias);
                        }
                    } else if (pending_tag[0] != '\0') {
                        save_typedef(pending_tag);
                    }
                    typedef_waiting = 0;
                    break;
                }
            }
            p++;
        }
        if (typedef_waiting && struct_depth > 0) {
            char* semi = strchr(line_ptr, ';');
            if (semi && !strchr(line_ptr, '{') && !strchr(line_ptr, '}')) {
                char tmp_line[1024];
                size_t copy_len = semi - line_ptr;
                if (copy_len >= sizeof(tmp_line)) copy_len = sizeof(tmp_line) - 1;
                strncpy(tmp_line, line_ptr, copy_len);
                tmp_line[copy_len] = '\0';
                char* last_space = strrchr(tmp_line, ' ');
                if (last_space) {
                    char* vars_str = last_space + 1;
                    char* token = strtok(vars_str, ",");
                    while (token) {
                        while (*token && (isspace((unsigned char)*token) || *token == '*')) token++;
                        if (*token) {
                            char* end_token = token;
                            while (*end_token && (isalnum((unsigned char)*end_token) || *end_token == '_')) end_token++;
                            size_t nlen = end_token - token;
                            if (nlen > 0) {
                                char* var_name = malloc(nlen + 1);
                                if (var_name) {
                                    strncpy(var_name, token, nlen);
                                    var_name[nlen] = '\0';
                                    save_variables(var_name);
                                    free(var_name);
                                }
                            }
                        }
                        token = strtok(NULL, ",");
                    }
                }
            }
        }
        return;
    }
    if (strncmp(line_ptr, "#define", 7) == 0) {
        char* define_ptr = line_ptr + 7;

        while (*define_ptr && isspace((unsigned char)*define_ptr)) {
            define_ptr++;
        }

        if (*define_ptr && (isalpha((unsigned char)*define_ptr) || *define_ptr == '_')) {
            char* macro_name_start = define_ptr;
            char* macro_name_end = macro_name_start;
            while (*macro_name_end && (isalnum((unsigned char)*macro_name_end) || *macro_name_end == '_')) {
                macro_name_end++;
            }

            if (macro_name_end > macro_name_start) {
                int name_len = macro_name_end - macro_name_start;
                char* macro_name = malloc(name_len + 1);
                if (macro_name) {
                    strncpy(macro_name, macro_name_start, name_len);
                    macro_name[name_len] = '\0';
                    save_variables(macro_name);
                    free(macro_name);
                }
            }
        }
    }

    for (int i = 0; i < C_PRIMITIVE_TYPES_SIZE; i++) {
        char* type_pos = working_line;
        while ((type_pos = strstr(type_pos, c_primitive_types[i])) != NULL) {
            bool is_start = (type_pos == working_line || (!isalnum(*(type_pos-1)) && *(type_pos-1) != '_'));
            bool is_end = (!*(type_pos + strlen(c_primitive_types[i])) || (!isalnum(*(type_pos + strlen(c_primitive_types[i]))) && *(type_pos + strlen(c_primitive_types[i])) != '_'));
            if (!is_start || !is_end) {
                type_pos++;
                continue;
            }
            char* after_type = type_pos + strlen(c_primitive_types[i]);
            
            while (*after_type && (isspace(*after_type) || *after_type == '*')) after_type++;
            
            while (*after_type && *after_type != ';') {
                while (*after_type && !isalnum(*after_type) && *after_type != '_') after_type++;
                if (!*after_type || *after_type == ';') break;

                char* var_start = after_type;
                char* var_end = var_start;

                while (*var_end && (isalnum(*var_end) || *var_end == '_')) var_end++;

                if (var_end > var_start) {
                    int name_len = var_end - var_start;
                    char* var_name = malloc(name_len + 1);
                    if (var_name) {
                        strncpy(var_name, var_start, name_len);
                        var_name[name_len] = '\0';

                        bool is_keyword = false;
                        for (int j = 0; j < blue_keywords_count; j++) {
                            if (strcmp(var_name, blue_keywords[j]) == 0) { is_keyword = true; break; }
                        }

                        if (!is_keyword) save_variables(var_name);
                        free(var_name);
                    }
                }
                after_type = var_end;
                while (*after_type && *after_type != ',' && *after_type != ';') after_type++;
                if (*after_type == ',') after_type++;
            }
            type_pos = after_type;
        }
    }
    for (size_t tt = 0; tt < typedefs_count; tt++) {
        char* type_pos = working_line;
        while ((type_pos = strstr(type_pos, typedefs[tt])) != NULL) {
            bool is_start = (type_pos == working_line
                 || (!isalnum(*(type_pos-1)) && *(type_pos-1) != '_'));

            bool is_end = (!*(type_pos + strlen(typedefs[tt]))
             || (!isalnum(*(type_pos + strlen(typedefs[tt])))
              && *(type_pos + strlen(typedefs[tt])) != '_'));

            if (!is_start || !is_end) {
                type_pos++;
                continue;
            }

            char* after_type = type_pos + strlen(typedefs[tt]);
            while (*after_type && (isspace((unsigned char)*after_type) || *after_type == '*')) after_type++;
            while (*after_type && *after_type != ';') {
                while (*after_type && !isalnum((unsigned char)*after_type) && *after_type != '_') after_type++;
                if (!*after_type || *after_type == ';') break;
                char* var_start = after_type;
                char* var_end = var_start;
                while (*var_end && (isalnum((unsigned char)*var_end) || *var_end == '_')) var_end++;
                if (var_end > var_start) {
                    int name_len = var_end - var_start;
                    char* var_name = malloc(name_len + 1);
                    if (var_name) {
                        strncpy(var_name, var_start, name_len);
                        var_name[name_len] = '\0';
                        save_variables(var_name);
                        free(var_name);
                    }
                }
                after_type = var_end;
                while (*after_type && *after_type != ',' && *after_type != ';') after_type++;
                if (*after_type == ',') after_type++;
            }
            type_pos = after_type;
        }
    }
}

void cleanup_variables() {
    if (variables) {
        for (size_t i = 0; i < variables_count; i++) {
            free(variables[i]);
        }
        free(variables);
        variables = NULL;
    }
    cleanup_typedefs();
}
