#pragma once

#include "../libs/sokol/sokol_gfx.h"
#include "../libs/sokol/linmath.h"
#include <stdint.h>

typedef struct {
    mat4x4 mvp;
} vs_params_t;

typedef enum {
    PIPELINE_STANDARD,
    PIPELINE_GRID,
    PIPELINE_COUNT
} PipelineType;


typedef struct {
    PipelineType type;
    sg_pipeline pipeline;
} RenderHandle;

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
    int index_count;
} Mesh;

typedef struct {
    vec3 eye;
    vec3 center;
    vec3 up;
    mat4x4 view;
    mat4x4 proj;
    float distance;
    float pitch;
    float yaw;
    bool mouse_down;
    float last_mouse_x;
    float last_mouse_y;
} Camera;

typedef struct {
    uint32_t id;
    // component pools
    Transform* transform;
    Mesh* mesh;
    RenderHandle* render;
} Entity;

typedef struct {
    Entity entities[1000];
    uint32_t entity_count;
    uint32_t next_entity_id;

    // Component pools
    Transform transforms[1000];
    Mesh meshes[1000];
    Camera camera;
    RenderHandle renders[1000];
    sg_pipeline pipelines[1000];
} World;

Entity* create_entity(World* world);
Transform* add_transform(World* world, Entity* entity, float x, float y, float z);
Mesh* add_cube_mesh(World* world, Entity* entity);
Mesh* add_grid_mesh(World* world, Entity* entity);
Entity* create_cube_entity(World* world, float x, float y, float z);
Entity* create_grid_entity(World* world);
void input_system(World* world);
void update_camera_system(World* world);
void render(World* world, mat4x4 view, mat4x4 proj);
Entity* create_cube(World* world, float x, float y, float z);
Entity* create_grid(World* world);
