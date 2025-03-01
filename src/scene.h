// scene.h
#pragma once

#include "types.h"

#define SCENE_VERSION 1

void clear_scene(World* world);
bool load_scene(World* world, const char* filename);
bool save_scene(World* world, const char* filename);
