#pragma once

#include "types.h"
#include "../libs/linmath/linmath.h"

void render_entities(World* world, mat4x4 view, mat4x4 proj);
void render_cameras(World* world, mat4x4 view, mat4x4 proj);
void render_grid(World* world, mat4x4 view, mat4x4 proj);


void render_init(void);
void render_text(float x, float y, const char* text);
void render_text_colored(float x, float y, const char* text, uint8_t r, uint8_t g, uint8_t b);
