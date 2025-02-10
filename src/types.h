#pragma once

#include <stdio.h>
#include "../libs/sokol/sokol_gfx.h"
#include "../libs/sokol/sokol_glue.h"
#include "../libs/sokol/linmath.h"

typedef struct {
    mat4x4 mvp;
} vs_params_t;

typedef struct Mesh {
    sg_buffer vertex_buffer;
    sg_buffer index_buffer;
    sg_bindings bindings;
    int index_count;
    int vertex_count;
} Mesh;

typedef struct Material {
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

typedef struct Entity {
    uint32_t id;
    
    Transform transform;

    // component pools
    Mesh* mesh;
    Renderable* renderable;
} Entity;

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

typedef struct World {
    Entity entities[1000];
    uint32_t entity_count;
    uint32_t next_entity_id;

    Mesh meshes[1000];
    uint32_t mesh_count;
    Material materials[1000];
    uint32_t material_count;
    
    // maybe move camera stuff to another struct
    Camera cameras[100];
    int camera_count;
    uint32_t next_camera_id;
    Camera camera;
    CameraControls control;

    // maybe move state to another struct too
    bool in_edit_mode;
    
    sg_pipeline pipelines[1000];

    Renderable renderables[1000];
    uint32_t renderable_count;
    struct nk_context* ctx;
} World;
