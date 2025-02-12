#pragma once

#include "types.h"
#include "../libs/linmath/linmath.h"

void render_entities(World* world, mat4x4 view, mat4x4 proj);
void render_cameras(World* world, mat4x4 view, mat4x4 proj);
void render_grid(World* world, mat4x4 view, mat4x4 proj);
