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

typedef struct {
    mat4x4 mvp;
} vs_params_t;

/* void create_gizmo(void) { */
/*     const float axis_length = 5.0f; */
/*     const float radius = 0.05f; */
/*     const int segments = 8; */
/*     float vertices[144 * 6];  // (segments * 6 vertices * 3 axes) * 6 floats per vertex */
/*     int vertex_count = 0; */

/*     // X axis (red) cylinder */
/*     for(int i = 0; i < segments; i++) { */
/*         float angle1 = (float)i * 2.0f * PI / segments; */
/*         float angle2 = (float)(i + 1) * 2.0f * PI / segments; */
        
/*         float y1 = cosf(angle1) * radius; */
/*         float z1 = sinf(angle1) * radius; */
/*         float y2 = cosf(angle2) * radius; */
/*         float z2 = sinf(angle2) * radius; */

/*         // First triangle */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = y1; */
/*         vertices[vertex_count++] = z1; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         vertices[vertex_count++] = axis_length; */
/*         vertices[vertex_count++] = y1; */
/*         vertices[vertex_count++] = z1; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = y2; */
/*         vertices[vertex_count++] = z2; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         // Second triangle */
/*         vertices[vertex_count++] = axis_length; */
/*         vertices[vertex_count++] = y1; */
/*         vertices[vertex_count++] = z1; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         vertices[vertex_count++] = axis_length; */
/*         vertices[vertex_count++] = y2; */
/*         vertices[vertex_count++] = z2; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = y2; */
/*         vertices[vertex_count++] = z2; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*     } */

/*     // Y axis (green) cylinder */
/*     for(int i = 0; i < segments; i++) { */
/*         float angle1 = (float)i * 2.0f * PI / segments; */
/*         float angle2 = (float)(i + 1) * 2.0f * PI / segments; */
        
/*         float x1 = cosf(angle1) * radius; */
/*         float z1 = sinf(angle1) * radius; */
/*         float x2 = cosf(angle2) * radius; */
/*         float z2 = sinf(angle2) * radius; */

/*         // First triangle */
/*         vertices[vertex_count++] = x1; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = z1; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         vertices[vertex_count++] = x1; */
/*         vertices[vertex_count++] = axis_length; */
/*         vertices[vertex_count++] = z1; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         vertices[vertex_count++] = x2; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = z2; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         // Second triangle */
/*         vertices[vertex_count++] = x1; */
/*         vertices[vertex_count++] = axis_length; */
/*         vertices[vertex_count++] = z1; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         vertices[vertex_count++] = x2; */
/*         vertices[vertex_count++] = axis_length; */
/*         vertices[vertex_count++] = z2; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */

/*         vertices[vertex_count++] = x2; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = z2; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*     } */

/*     // Z axis (blue) cylinder */
/*     for(int i = 0; i < segments; i++) { */
/*         float angle1 = (float)i * 2.0f * PI / segments; */
/*         float angle2 = (float)(i + 1) * 2.0f * PI / segments; */
        
/*         float x1 = cosf(angle1) * radius; */
/*         float y1 = sinf(angle1) * radius; */
/*         float x2 = cosf(angle2) * radius; */
/*         float y2 = sinf(angle2) * radius; */

/*         // First triangle */
/*         vertices[vertex_count++] = x1; */
/*         vertices[vertex_count++] = y1; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */

/*         vertices[vertex_count++] = x1; */
/*         vertices[vertex_count++] = y1; */
/*         vertices[vertex_count++] = axis_length; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */

/*         vertices[vertex_count++] = x2; */
/*         vertices[vertex_count++] = y2; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */

/*         // Second triangle */
/*         vertices[vertex_count++] = x1; */
/*         vertices[vertex_count++] = y1; */
/*         vertices[vertex_count++] = axis_length; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */

/*         vertices[vertex_count++] = x2; */
/*         vertices[vertex_count++] = y2; */
/*         vertices[vertex_count++] = axis_length; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */

/*         vertices[vertex_count++] = x2; */
/*         vertices[vertex_count++] = y2; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 0.0f; */
/*         vertices[vertex_count++] = 1.0f; */
/*     } */

/*     state.gizmo_bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){ */
/*         .data = (sg_range){vertices, vertex_count * sizeof(float)}, */
/*         .label = "gizmo-vertices" */
/*     }); */

/*     char* vs = read_shader_file("src/shaders/grid_vs.metal"); */
/*     char* fs = read_shader_file("src/shaders/grid_fs.metal"); */

/*     sg_shader shd = sg_make_shader(&(sg_shader_desc){ */
/*         .uniform_blocks[0] = { */
/*             .stage = SG_SHADERSTAGE_VERTEX, */
/*             .size = sizeof(vs_params_t), */
/*             .msl_buffer_n = 0, */
/*         }, */
/*         .vertex_func = { */
/*             .source = vs, */
/*         }, */
/*         .fragment_func = { */
/*             .source = fs, */
/*         } */
/*     }); */

/*     state.gizmo_pip = sg_make_pipeline(&(sg_pipeline_desc){ */
/*         .layout = { */
/*             .buffers[0].stride = 6 * sizeof(float),  // pos + color */
/*             .attrs = { */
/*                 [0] = { .format = SG_VERTEXFORMAT_FLOAT3 },  // position */
/*                 [1] = { .format = SG_VERTEXFORMAT_FLOAT3 }   // color */
/*             } */
/*         }, */
/*         .shader = shd, */
/*         .primitive_type = SG_PRIMITIVETYPE_TRIANGLES, */
/*         .depth = { */
/*             .write_enabled = true, */
/*             .compare = SG_COMPAREFUNC_LESS_EQUAL, */
/*         }, */
/*         .label = "gizmo-pipeline" */
/*     }); */

/*     free(vs); */
/*     free(fs); */
/* } */

void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    memset(&world, 0, sizeof(World));
    
    create_cube_entity(&world, 0.0f, 0.0f, 0.0f);
    create_grid_entity(&world);

    // Create shader
    char* vs_source = read_shader_file("src/shaders/cube_vs.metal");
    char* fs_source = read_shader_file("src/shaders/cube_fs.metal");
    
    
    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0,
        },
        .vertex_func = { .source = vs_source },
        .fragment_func = { .source = fs_source }
    });

    // Create pipeline
    pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[0].stride = 28,  // 3 floats position + 4 floats color = 7 floats * 4 bytes
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
    });

    char* grid_vs_source = read_shader_file("src/shaders/grid_vs.metal");
    char* grid_fs_source = read_shader_file("src/shaders/grid_fs.metal");
    printf("Grid VS shader source:\n%s\n", grid_vs_source);
    printf("Grid FS shader source:\n%s\n", grid_fs_source);

    sg_shader grid_shd = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0,
        },
        .vertex_func = {.source = grid_vs_source },
        .fragment_func = {.source = grid_fs_source }
    });

    grid_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[0].stride = 6 * sizeof(float), // vec3 pos, vec3 color
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,
                [1].format = SG_VERTEXFORMAT_FLOAT3
            }
        },
        .shader = grid_shd,
        .primitive_type = SG_PRIMITIVETYPE_LINES, // Important: Use lines
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
         },
    });

    world.camera.distance = 6.0f;
    world.camera.pitch = 30.0f;
    world.camera.yaw = 0.0f;
    
    free(vs_source);
    free(fs_source);
}

void render(World* world, mat4x4 view, mat4x4 proj, sg_pipeline standard_pip, sg_pipeline grid_pip) {
    for (uint32_t i = 0; i < world->entity_count; i++) {
        Entity* entity = &world->entities[i];       
        
        if (entity->transform && entity->mesh) {

            // Setup model matrix and MVP
            mat4x4 model;
            mat4x4_identity(model);
            mat4x4_translate(model, 
                entity->transform->position[0],
                entity->transform->position[1],
                entity->transform->position[2]
            );
            
            mat4x4 mvp;
            mat4x4_mul(mvp, proj, view);
            mat4x4_mul(mvp, mvp, model);
            
            vs_params_t vs_params;
            memcpy(&vs_params.mvp, mvp, sizeof(mat4x4));
            
            // Choose pipeline based on whether it's indexed or not
            sg_pipeline pipeline = entity->mesh->index_buffer.id == 0 ? grid_pip : standard_pip;
            sg_apply_pipeline(pipeline);

            // Apply bindings based on whether we have an index buffer
            if (entity->mesh->index_buffer.id != 0) {
                sg_apply_bindings(&(sg_bindings){
                    .vertex_buffers[0] = entity->mesh->vertex_buffer,
                    .index_buffer = entity->mesh->index_buffer
                });
            } else {
                sg_apply_bindings(&(sg_bindings){
                    .vertex_buffers[0] = entity->mesh->vertex_buffer
                });
            }

            sg_apply_uniforms(0, &SG_RANGE(vs_params));
            sg_draw(0, entity->mesh->vertex_count, 1);
        }
    }
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

    render(&world, view, proj, pip, grid_pip);

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
