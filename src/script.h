#pragma once

#include "cJSON.h"

void init_script_pipe(void);
void check_script_pipe(World* world);
void add_script(ScriptQueue* queue, const char* filepath);
char* get_next_script(ScriptQueue* queue);
void load_script(World* world, const char* filepath);
void add_script_to_backlog(World* world, const char* script_path);
void play_next_line(World* world);
void load_existing_scripts(World* world);
