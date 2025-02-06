#pragma once

#include "../libs/sokol/sokol_gfx.h"
#include <stdint.h>

// Components
typedef struct {
    float position[3];
    float rotation[3];
    float scale[3];
} Transform;

typedef struct {
    sg_buffer vertex_buffer;
    sg_buffer index_buffer;
    int vertex_count;
} Mesh;

typedef struct {
    uint32_t id;
    Transform* transform;
    Mesh* mesh;
} Entity;

typedef struct {
    Entity entities[1000];
    uint32_t entity_count;
    uint32_t next_entity_id;

    // Component pools
    Transform transforms[1000];
    Mesh meshes[1000];
} World;

Entity* create_entity(World* world);
Transform* add_transform(World* world, Entity* entity, float x, float y, float z);
Mesh* add_cube_mesh(World* world, Entity* entity);
Entity* create_cube_entity(World* world, float x, float y, float z);
