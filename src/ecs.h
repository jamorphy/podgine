#pragma once

#include "../libs/sokol/sokol_gfx.h"
#include "../libs/sokol/sokol_glue.h"
#include "../libs/sokol/linmath.h"
#include <stdint.h>
#include "utils.h"

typedef struct {
    mat4x4 mvp;
} vs_params_t;

typedef struct {
    sg_buffer vertex_buffer;
    sg_buffer index_buffer;
    sg_bindings bindings;
    int index_count;
    int vertex_count;
} Mesh;

typedef struct {
    sg_shader shader;
    sg_pipeline pipeline;
} Material;

typedef struct {
    Mesh *mesh;
    Material *material;
} Renderable;

typedef struct {
    float position[3];
    float rotation[3];
    float scale[3];
} Transform;    

typedef enum {
    PIPELINE_STANDARD,
    PIPELINE_GRID,
    PIPELINE_COUNT,
    PIPELINE_SPRITE
} PipelineType;

typedef struct {
    PipelineType type;
    sg_pipeline pipeline;
} RenderHandle;

// Components

typedef struct {
    uint32_t id;
    vec3 eye;
    vec3 position;
    vec3 center;
    vec3 up;
    mat4x4 view;
    mat4x4 proj;
    float distance;
    float pitch;
    float yaw;
    char name[32];
} Camera;

typedef struct {
    bool mouse_down;
    float last_mouse_x;
    float last_mouse_y;
    Camera* active_camera;

    bool key_w, key_a, key_s, key_d;
} CameraControls;

typedef struct {
    uint32_t id;
    // component pools
    Transform* transform;
    Mesh* mesh;
    
    RenderHandle* render;
    Renderable* renderable;
} Entity;

#define EDITOR_CAMERA_INDEX 0

typedef struct {
    Entity entities[1000];
    uint32_t entity_count;
    uint32_t next_entity_id;

    // Component pools
    Transform transforms[1000];
    uint32_t transform_count;
    Mesh meshes[1000];
    uint32_t mesh_count;
    Material materials[1000];
    uint32_t material_count;
    
    // maybe move camera stuff to another struct
    Camera cameras[16];
    int camera_count;
    uint32_t next_camera_id;
    Camera camera;
    CameraControls control;

    // maybe move state to another struct too
    bool in_edit_mode;
    
    RenderHandle renders[1000];
    sg_pipeline pipelines[1000];

    Renderable renderables[1000];
    uint32_t renderable_count;
} World;


// move later

Mesh* create_cube_mesh(World* world);
Material* create_cube_material(World* world);
Entity* create_cube_new(World* world, float x, float y, float z);

char* read_shader_file(const char* filepath);
void render_entities(World* world, mat4x4 view, mat4x4 proj);

// move later

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
void create_camera(World* world,
                   float distance,
                   float pitch,
                   float yaw,
                   float x,
                   float y,
                   float z,
                   const char* name);
