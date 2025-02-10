#pragma once

#include "../libs/sokol/sokol_gfx.h"
#include "../libs/sokol/sokol_glue.h"
#include "../libs/sokol/linmath.h"
#include <stdint.h>

#include "types.h"

Entity* create_entity(World* world);
void destroy_entity(World* world, Entity* entity);
void destroy_mesh(Mesh* mesh);
void destroy_material(Material* material);



// move these some where else
#define EDITOR_CAMERA_INDEX 0
Mesh* create_cube_mesh(void);
Mesh* create_quad_mesh(sg_image texture);
Entity* create_img(World* world, const char* image_path, vec3 pos, vec3 scale);
Material* create_textured_material(void);
Material* create_cube_material(void);
Entity* create_cube(World* world, vec3 pos, vec3 scale);
char* read_shader_file(const char* filepath);
void render_entities(World* world, mat4x4 view, mat4x4 proj);
Mesh* add_grid_mesh(World* world, Entity* entity);
Entity* create_grid_entity(World* world);
Entity* create_grid(World* world);
