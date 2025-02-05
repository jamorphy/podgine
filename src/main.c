#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "linmath.h"
#include "utils.h"
#include "utils_math.h"

#define SOKOL_METAL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "state.h"
#include "gui.h"

#include "sokol_nuklear.h"

static struct state state;

typedef struct {
    mat4x4 mvp;
} vs_params_t;

void create_gizmo(void) {
    const float axis_length = 5.0f;
    const float radius = 0.05f;
    const int segments = 8;
    float vertices[144 * 6];  // (segments * 6 vertices * 3 axes) * 6 floats per vertex
    int vertex_count = 0;

    // X axis (red) cylinder
    for(int i = 0; i < segments; i++) {
        float angle1 = (float)i * 2.0f * PI / segments;
        float angle2 = (float)(i + 1) * 2.0f * PI / segments;
        
        float y1 = cosf(angle1) * radius;
        float z1 = sinf(angle1) * radius;
        float y2 = cosf(angle2) * radius;
        float z2 = sinf(angle2) * radius;

        // First triangle
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = y1;
        vertices[vertex_count++] = z1;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;

        vertices[vertex_count++] = axis_length;
        vertices[vertex_count++] = y1;
        vertices[vertex_count++] = z1;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;

        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = y2;
        vertices[vertex_count++] = z2;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;

        // Second triangle
        vertices[vertex_count++] = axis_length;
        vertices[vertex_count++] = y1;
        vertices[vertex_count++] = z1;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;

        vertices[vertex_count++] = axis_length;
        vertices[vertex_count++] = y2;
        vertices[vertex_count++] = z2;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;

        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = y2;
        vertices[vertex_count++] = z2;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
    }

    // Y axis (green) cylinder
    for(int i = 0; i < segments; i++) {
        float angle1 = (float)i * 2.0f * PI / segments;
        float angle2 = (float)(i + 1) * 2.0f * PI / segments;
        
        float x1 = cosf(angle1) * radius;
        float z1 = sinf(angle1) * radius;
        float x2 = cosf(angle2) * radius;
        float z2 = sinf(angle2) * radius;

        // First triangle
        vertices[vertex_count++] = x1;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = z1;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;

        vertices[vertex_count++] = x1;
        vertices[vertex_count++] = axis_length;
        vertices[vertex_count++] = z1;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;

        vertices[vertex_count++] = x2;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = z2;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;

        // Second triangle
        vertices[vertex_count++] = x1;
        vertices[vertex_count++] = axis_length;
        vertices[vertex_count++] = z1;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;

        vertices[vertex_count++] = x2;
        vertices[vertex_count++] = axis_length;
        vertices[vertex_count++] = z2;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;

        vertices[vertex_count++] = x2;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = z2;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;
        vertices[vertex_count++] = 0.0f;
    }

    // Z axis (blue) cylinder
    for(int i = 0; i < segments; i++) {
        float angle1 = (float)i * 2.0f * PI / segments;
        float angle2 = (float)(i + 1) * 2.0f * PI / segments;
        
        float x1 = cosf(angle1) * radius;
        float y1 = sinf(angle1) * radius;
        float x2 = cosf(angle2) * radius;
        float y2 = sinf(angle2) * radius;

        // First triangle
        vertices[vertex_count++] = x1;
        vertices[vertex_count++] = y1;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;

        vertices[vertex_count++] = x1;
        vertices[vertex_count++] = y1;
        vertices[vertex_count++] = axis_length;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;

        vertices[vertex_count++] = x2;
        vertices[vertex_count++] = y2;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;

        // Second triangle
        vertices[vertex_count++] = x1;
        vertices[vertex_count++] = y1;
        vertices[vertex_count++] = axis_length;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;

        vertices[vertex_count++] = x2;
        vertices[vertex_count++] = y2;
        vertices[vertex_count++] = axis_length;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;

        vertices[vertex_count++] = x2;
        vertices[vertex_count++] = y2;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 0.0f;
        vertices[vertex_count++] = 1.0f;
    }

    state.gizmo_bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = (sg_range){vertices, vertex_count * sizeof(float)},
        .label = "gizmo-vertices"
    });

    char* vs = read_shader_file("src/shaders/grid_vs.metal");
    char* fs = read_shader_file("src/shaders/grid_fs.metal");

    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0,
        },
        .vertex_func = {
            .source = vs,
        },
        .fragment_func = {
            .source = fs,
        }
    });

    state.gizmo_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[0].stride = 6 * sizeof(float),  // pos + color
            .attrs = {
                [0] = { .format = SG_VERTEXFORMAT_FLOAT3 },  // position
                [1] = { .format = SG_VERTEXFORMAT_FLOAT3 }   // color
            }
        },
        .shader = shd,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
        },
        .label = "gizmo-pipeline"
    });

    free(vs);
    free(fs);
}


void create_grid(void) {
    // Let's make a 20x20 grid, centered at origin (-10 to +10)
    const int GRID_SIZE = 20;  // This means -10 to +10, giving us 21 lines in each direction
    const float GRID_SPACE = 2.0f;
    const int LINES_PER_DIR = (GRID_SIZE * 2 + 1);
    const int VERTS_PER_LINE = 2;
    const int TOTAL_LINES = LINES_PER_DIR * 2;  // Lines in both X and Z directions
    const int VERTEX_COUNT = TOTAL_LINES * VERTS_PER_LINE;
    
    // Vertices and colors (xyz + rgb per vertex)
    float* vertices = (float*)malloc(VERTEX_COUNT * 6 * sizeof(float));
    int vertex_idx = 0;

    // Create grid lines
    for (int i = -GRID_SIZE; i <= GRID_SIZE; i++) {
        float color_intensity = (i == 0) ? 1.0f : 0.25f;  // Brighter color for center lines
        
        // Lines along X axis
        // First vertex
        vertices[vertex_idx++] = i * GRID_SPACE;  // x
        vertices[vertex_idx++] = -0.01f;            // y
        vertices[vertex_idx++] = -GRID_SIZE * GRID_SPACE; // z
        vertices[vertex_idx++] = color_intensity; // r
        vertices[vertex_idx++] = color_intensity; // g
        vertices[vertex_idx++] = color_intensity; // b
        
        // Second vertex
        vertices[vertex_idx++] = i * GRID_SPACE;  // x
        vertices[vertex_idx++] = 0.0f;            // y
        vertices[vertex_idx++] = GRID_SIZE * GRID_SPACE;  // z
        vertices[vertex_idx++] = color_intensity; // r
        vertices[vertex_idx++] = color_intensity; // g
        vertices[vertex_idx++] = color_intensity; // b

        // Lines along Z axis
        // First vertex
        vertices[vertex_idx++] = -GRID_SIZE * GRID_SPACE; // x
        vertices[vertex_idx++] = 0.0f;            // y
        vertices[vertex_idx++] = i * GRID_SPACE;  // z
        vertices[vertex_idx++] = color_intensity; // r
        vertices[vertex_idx++] = color_intensity; // g
        vertices[vertex_idx++] = color_intensity; // b
        
        // Second vertex
        vertices[vertex_idx++] = GRID_SIZE * GRID_SPACE;  // x
        vertices[vertex_idx++] = 0.0f;            // y
        vertices[vertex_idx++] = i * GRID_SPACE;  // z
        vertices[vertex_idx++] = color_intensity; // r
        vertices[vertex_idx++] = color_intensity; // g
        vertices[vertex_idx++] = color_intensity; // b
    }

    state.grid_bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = (sg_range){vertices, vertex_idx * sizeof(float)},
        .label = "grid-vertices"
    });

    free(vertices);

    char* grid_vs = read_shader_file("src/shaders/grid_vs.metal");
    char* grid_fs = read_shader_file("src/shaders/grid_fs.metal");
    if (!grid_vs || !grid_fs) {
        return;
    }

    // Update the grid shader to handle vertex colors
    sg_shader grid_shd = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0,
        },
        .vertex_func = {
            .source = grid_vs,
            //.entry = "grid_vs"
        },
        .fragment_func = {
            .source = grid_fs,
            //.entry = "grid_fs"
        }
    });

    // Update pipeline to include vertex color attribute
    state.grid_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[0].stride = 6 * sizeof(float),  // xyz + rgb = 6 floats
            .attrs = {
                [0] = { .format = SG_VERTEXFORMAT_FLOAT3 },  // position
                [1] = { .format = SG_VERTEXFORMAT_FLOAT3 }   // color
            }
        },
        .shader = grid_shd,
        .primitive_type = SG_PRIMITIVETYPE_LINES,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        },
        .label = "grid-pipeline"
    });

    free(grid_vs);
    free(grid_fs);
}


void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    snk_setup(&(snk_desc_t){
        .enable_set_mouse_cursor = false,
    });

    nk_style_hide_cursor(snk_new_frame());

    /* cube vertex buffer */
    float vertices[] = {
        -1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
         1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
         1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
        -1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,

        -1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
         1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
         1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
        -1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,

        -1.0, -1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0,  1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0,  1.0,  1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0, -1.0,  1.0,   0.0, 0.0, 1.0, 1.0,

        1.0, -1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
        1.0,  1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
        1.0,  1.0,  1.0,    1.0, 0.5, 0.0, 1.0,
        1.0, -1.0,  1.0,    1.0, 0.5, 0.0, 1.0,

        -1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,
        -1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
         1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
         1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,

        -1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0,
        -1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
         1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
         1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0
    };
    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .label = "cube-vertices"
    });

    /* create an index buffer for the cube */
    uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };
    sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "cube-indices"
    });

    char* cube_vs = read_shader_file("src/shaders/cube_vs.metal");
    char* cube_fs = read_shader_file("src/shaders/cube_fs.metal");
    if (!cube_vs || !cube_fs) {
        return;
    }

    /* create shader */
    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0,
        },
        .vertex_func = {
            .source = cube_vs,
        },
        .fragment_func = {
            .source = cube_fs,
        }
    });

    state.camera_distance = 6.0f;
    state.camera_pitch = 30.0f;  // Initial pitch angle in degrees
    state.camera_yaw = 0.0f;     // Initial yaw angle in degrees
    state.mouse_down = false;

    /* create pipeline object */
    state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[0].stride = 28,
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,
                [1].format = SG_VERTEXFORMAT_FLOAT4
            }
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
        },
        .label = "cube-pipeline"
    });

    /* setup resource bindings */
    state.bind = (sg_bindings) {
        .vertex_buffers[0] = vbuf,
        .index_buffer = ibuf
    };

    create_grid();
    state.cube_position[0] = 0.0f;
    state.cube_position[1] = 0.5f;
    state.cube_position[2] = 0.0f;
    create_gizmo();

    free(cube_vs);
    free(cube_fs);
}


void input(const sapp_event* ev) {
    switch (ev->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN: {

            if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
                state.mouse_down = true;
                state.last_mouse_x = ev->mouse_x;
                state.last_mouse_y = ev->mouse_y;
            }
        } break;

        case SAPP_EVENTTYPE_MOUSE_UP: {
            if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
                state.mouse_down = false;
            }
        } break;

        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            state.camera_distance -= ev->scroll_y;
            state.camera_distance = clamp(2.0f, state.camera_distance, 20.0f);
        } break;

        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            if (state.mouse_down) {
                float delta_x = ev->mouse_x - state.last_mouse_x;
                float delta_y = ev->mouse_y - state.last_mouse_y;

                state.camera_yaw += -delta_x * 0.3f;
                state.camera_pitch += delta_y * 0.3f;                
                
                state.camera_pitch = clamp(state.camera_pitch, -89.0f, 89.0f);

                state.last_mouse_x = ev->mouse_x;
                state.last_mouse_y = ev->mouse_y;
            }
        } break;

        default: break;
    }
}


void frame(void) {

    state.nk_ctx = snk_new_frame();
    
    const float w = sapp_widthf();
    const float h = sapp_heightf();

    float normalized_yaw = fmodf(state.camera_yaw, 360.0f);
    if (normalized_yaw < 0) normalized_yaw += 360.0f;

    // Convert orbit angles to camera position
    float pitch_rad = TO_RAD(state.camera_pitch);
    float yaw_rad = TO_RAD(normalized_yaw);

    // Calculate camera position using spherical coordinates
    float cam_x = state.camera_distance * cosf(pitch_rad) * sinf(yaw_rad);
    float cam_y = state.camera_distance * sinf(pitch_rad);
    float cam_z = state.camera_distance * cosf(pitch_rad) * cosf(yaw_rad);

    mat4x4 proj;
    mat4x4 view;
    mat4x4_perspective(proj, TO_RAD(60.0f), w/h, 0.1f, 500.0f);
    
    vec3 eye = {cam_x, cam_y, cam_z};
    vec3 center = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 1.0f, 0.0f};
    mat4x4_look_at(view, eye, center, up);

    vs_params_t vs_params;

    sg_begin_pass(&(sg_pass){
            .action = {
                .colors[0] = {
                    .load_action = SG_LOADACTION_CLEAR,
                    .clear_value = { 0.25f, 0.5f, 0.75f, 1.0f }
                },
                .depth = {
                    .load_action = SG_LOADACTION_CLEAR,
                    .clear_value = 1.0f
                }
            },
            .swapchain = sglue_swapchain()
        });

    mat4x4 model;
    mat4x4_identity(model);
    vec3 trans = {0.0f, 0.5f, 0.0f};
    mat4x4_translate(model, trans[0], trans[1], trans[2]);
    
    mat4x4 mvp;
    mat4x4_mul(mvp, proj, view);
    mat4x4_mul(mvp, mvp, model);
    memcpy(&vs_params.mvp, mvp, sizeof(mat4x4));

    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_draw(0, 36, 1);

    // For grid
    mat4x4_mul(mvp, proj, view);
    memcpy(&vs_params.mvp, mvp, sizeof(mat4x4));
    sg_apply_pipeline(state.grid_pip);
    sg_apply_bindings(&state.grid_bind);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_draw(0, (20 * 2 + 1) * 4, 1);

    // gizmo
    mat4x4 gizmo_model;
    mat4x4_identity(gizmo_model);;
    mat4x4_translate(gizmo_model, 
        state.cube_position[0], 
        state.cube_position[1], 
        state.cube_position[2]);
    
    mat4x4_mul(mvp, proj, view);
    mat4x4_mul(mvp, mvp, gizmo_model);
    memcpy(&vs_params.mvp, mvp, sizeof(mat4x4));

    sg_apply_pipeline(state.gizmo_pip);
    sg_apply_bindings(&state.gizmo_bind);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_draw(0, 144, 1);  // 3 axes * 2 vertices each = 6 vertices

    snk_new_frame();
    draw_nuklear_gui(&state);
    snk_render(sapp_width(), sapp_height()); 

    sg_end_pass();
    sg_commit();
}


void cleanup(void) {
    snk_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    int WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 960;
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = input,
        .width = WINDOW_WIDTH,
        .height = WINDOW_HEIGHT,
        .sample_count = 4,
        .window_title = "Cube",
        .icon.sokol_default = false,
        .swap_interval = 1
    };
}
