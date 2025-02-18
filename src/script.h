#pragma once

#include "cJSON.h"

void load_script(Script* script, const char* filepath);
void add_script_to_backlog(World* world, const char* script_path);
void play_next_line(World* world);
void load_existing_scripts(World* world);
