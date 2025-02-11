// scene.h
#pragma once

#include "types.h"
#include <stdbool.h>

// Current scene file version
#define SCENE_VERSION 1

// Scene management functions
void clear_scene(World* world);
bool save_scene(World* world, const char* filename);
bool load_scene(World* world, const char* filename);
