#pragma once

#include <stdio.h>
#include "../libs/sokol/sokol_gfx.h"
#include "../libs/sokol/sokol_glue.h"
#include "../libs/linmath/linmath.h"
#include "../libs/nuklear/nuklear.h"
#include "constants.h"

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

typedef struct Transform {
    vec3 position;
    vec3 rotation;
    vec3 scale;
} Transform;

typedef struct {
    uint32_t id;
    vec3 front;
    vec3 eye;
    vec3 position;
    vec3 center;
    vec3 up;
    mat4x4 view;
    mat4x4 proj;
    //float distance;
    float pitch;
    float yaw;
    vec3 forward;
    vec3 right;
    char name[32];
} Camera;

typedef struct {
    bool mouse_down;
    float last_mouse_x;
    float last_mouse_y;
    Camera* active_camera;

    bool key_w, key_a, key_s, key_d, key_spc, key_shift;
} CameraControls;

typedef struct Character {
    uint32_t id;
    const char* character_id;
    const char* display_name;
    Camera* cam;
} Character;

typedef struct Entity {
    uint32_t id;
    Transform transform;
    Renderable* renderable;
    Character* character;
} Entity;

typedef struct {
    char* character;
    char* text;
    char* audio_file;
} Line;

typedef struct {
    Line* lines;
    int line_count;
} Script;

// TODO: should i stop using fixed arrays
typedef struct World {
    Entity entities[1000];
    uint32_t entity_count;
    uint32_t next_entity_id;
    
    // TODO: maybe move camera stuff to another struct
    Camera cameras[MAX_CAMERAS];
    int camera_count;
    uint32_t next_camera_id;
    Camera active_camera;
    CameraControls control;

    // TODO: maybe move state to another struct too
    bool in_edit_mode;
    nk_bool show_grid;

    Renderable camera_visualization_renderable;
    Renderable grid_renderable;

    Renderable renderables[1000];
    uint32_t renderable_count;
    struct nk_context* ctx;

    Script* script;

    // TODO: HACKY INCOMING
    bool is_playing_audio;
    int current_line;  // to track which line we're on
    char* current_speaking_character;
    // TODO: END HACKY

    bool quit;
} World;
