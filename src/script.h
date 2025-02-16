#pragma once

#include "cJSON.h"

void parse_script(World* world, cJSON *json);
void play_script(World* world);
void play_next_line(World* world);
