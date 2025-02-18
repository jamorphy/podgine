#pragma once

#include "../libs/sokol/sokol_gfx.h"
#include "../libs/sokol/sokol_glue.h"
#include "../libs/linmath/linmath.h"

#include "types.h"

Entity* create_entity(World* world);
void destroy_entity(World* world, Entity* entity);
void destroy_mesh(Mesh* mesh);
void destroy_material(Material* material);

void create_and_set_grid(Renderable *renderable);
void destroy_grid(Renderable *renderable);

// move these some where else
Mesh* create_quad_mesh(sg_image texture);
Material* create_textured_material(void);
Entity* create_img(World* world, const char* image_path, vec3 pos, vec3 scale);

Mesh* create_cube_mesh(void);
Material* create_cube_material(void);
Entity* create_cube(World* world, vec3 pos, vec3 scale);

void entity_set_transform_xyz(Entity* entity, vec3 xyz);
void entity_set_transform_rot(Entity* entity, vec3 rot);
void entity_set_transform_scale(Entity* entity, vec3 scale);
