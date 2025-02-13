#pragma once

#include "types.h"
#include "ecs.h"

Entity* create_character(World* world, const char* image_path, const char* character_id, const char* display_name);
