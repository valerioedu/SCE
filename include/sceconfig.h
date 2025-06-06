#ifndef SCECONFIG_H
#define SCECONFIG_H

#include "library.h"

void config_editor();
void load_config(EditorConfig *config);
void save_config(const EditorConfig *config);

#endif