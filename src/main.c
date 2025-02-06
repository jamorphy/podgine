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

#include "ecs.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_COMMAND_USERDATA
#include "nuklear.h"
#include "sokol_nuklear.h"

sg_pass_action pass_action;
sg_pipeline pip;
sg_bindings bind;
World world;
sg_pipeline grid_pip;

void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    memset(&world, 0, sizeof(World));

    char* vs_source = read_shader_file("src/shaders/cube_vs.metal");
    char* fs_source = read_shader_file("src/shaders/cube_fs.metal");

    sg_shader standard_shader = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0,
        },
        .vertex_func = { .source = vs_source },
        .fragment_func = { .source = fs_source }
    });

    world.pipelines[PIPELINE_STANDARD] = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[0].stride = 28,
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,
                [1].format = SG_VERTEXFORMAT_FLOAT4
            }
        },
        .shader = standard_shader,
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
        },
    });

    char* grid_vs_source = read_shader_file("src/shaders/grid_vs.metal");
    char* grid_fs_source = read_shader_file("src/shaders/grid_fs.metal");

    sg_shader grid_shader = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0,
        },
        .vertex_func = { .source = grid_vs_source },
        .fragment_func = { .source = grid_fs_source }
    });

    world.pipelines[PIPELINE_GRID] = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[0].stride = 6 * sizeof(float),
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,
                [1].format = SG_VERTEXFORMAT_FLOAT3
            }
        },
        .shader = grid_shader,
        .primitive_type = SG_PRIMITIVETYPE_LINES,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
        },
    });
    
    create_cube(&world, 0.0f, 0.0f, 0.0f);
    create_grid(&world);

    world.camera.distance = 6.0f;
    world.camera.pitch = 30.0f;
    world.camera.yaw = 0.0f;

    free(vs_source);
    free(fs_source);
}

void input(const sapp_event* ev) {
    switch (ev->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN: {

            if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
                world.camera.mouse_down = true;
                world.camera.last_mouse_x = ev->mouse_x;
                world.camera.last_mouse_y = ev->mouse_y;
            }
        } break;

        case SAPP_EVENTTYPE_MOUSE_UP: {
            if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
                world.camera.mouse_down = false;
            }
        } break;

        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            world.camera.distance -= ev->scroll_y;
            world.camera.distance = clamp(2.0f, world.camera.distance, 20.0f);
        } break;

        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            if (world.camera.mouse_down) {
                float delta_x = ev->mouse_x - world.camera.last_mouse_x;
                float delta_y = ev->mouse_y - world.camera.last_mouse_y;

                world.camera.yaw += -delta_x * 0.3f;
                world.camera.pitch += delta_y * 0.3f;                
                
                world.camera.pitch = clamp(world.camera.pitch, -89.0f, 89.0f);

                world.camera.last_mouse_x = ev->mouse_x;
                world.camera.last_mouse_y = ev->mouse_y;
            }
        } break;

        default: break;
    }
}

void frame(void) {

    const float w = sapp_widthf();
    const float h = sapp_heightf();

    float normalized_yaw = fmodf(world.camera.yaw, 360.0f);
    if (normalized_yaw < 0) normalized_yaw += 360.0f;

    float pitch_rad = TO_RAD(world.camera.pitch);
    float yaw_rad = TO_RAD(normalized_yaw);

    float cam_x = world.camera.distance * cosf(pitch_rad) * sinf(yaw_rad);
    float cam_y = world.camera.distance * sinf(pitch_rad);
    float cam_z = world.camera.distance * cosf(pitch_rad) * cosf(yaw_rad);

    mat4x4 proj;
    mat4x4 view;
    mat4x4_perspective(proj, TO_RAD(60.0f), w/h, 0.1f, 500.0f);
    
    vec3 eye = {cam_x, cam_y, cam_z};
    vec3 center = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 1.0f, 0.0f};
    mat4x4_look_at(view, eye, center, up);

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

    render(&world, view, proj);

    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    snk_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    int WINDOW_WIDTH = 1024, WINDOW_HEIGHT = 768;
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
