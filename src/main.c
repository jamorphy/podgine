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

#define NUKLEAR_IMPLEMENTATION
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

sg_pass_action pass_action;
sg_pipeline pip;
sg_bindings bind;
World world;
sg_pipeline grid_pip;
struct nk_context* nk_ctx;

// img stuff
sg_image my_img;
sg_bindings quad_bind = {0};

static const float quad_vertices[] = {
    // positions         // UVs
    -0.5f, -0.5f, 0.0f,   0.0f, 1.0f,
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f,
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,   0.0f, 0.0f,
};

static const uint16_t quad_indices[] = {
    0, 1, 2,
    0, 2, 3
};

void create_textured_quad(void) {
    quad_bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(quad_vertices),
        .label = "quad-vertices"
    });
    quad_bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(quad_indices),
        .label = "quad-indices"
    });
    // Bind the texture to slot 0 (it must match the shader binding)
    quad_bind.images[0] = my_img;
    // Create a simple sampler
    quad_bind.samplers[0] = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
    });
}

sg_pipeline tex_pip;



void create_tex_pipeline(void) {

    sg_shader tex_shader = sg_make_shader(&(sg_shader_desc){
    // (1) tell Sokol about the uniform block:
    .uniform_blocks[0] = {
        .stage = SG_SHADERSTAGE_VERTEX,
        .size  = sizeof(vs_params_t),
        .msl_buffer_n = 0,
    },

    // (2) tell Sokol about the *fragment texture*:
    .images[0] = {
        .stage         = SG_SHADERSTAGE_FRAGMENT,
        .msl_texture_n = 0  // matches [[texture(0)]]
    },
    // optionally declare the sampler if you want a distinct binding
    .samplers[0] = {
        .stage         = SG_SHADERSTAGE_FRAGMENT,
        .msl_sampler_n = 0  // matches [[sampler(0)]]
    },
    // (3) pair them up (optional in many cases, but recommended):
    .image_sampler_pairs[0] = {
        .stage        = SG_SHADERSTAGE_FRAGMENT,
        .image_slot   = 0,
        .sampler_slot = 0
    },

    // (4) supply the MSL code for vertex/fragment:
    .vertex_func = {
        .entry  = "vs_main",
        .source = "#include <metal_stdlib>\n"
            "using namespace metal;\n"
            "struct vs_in {\n"
            "    float3 pos [[attribute(0)]];\n"
            "    float2 uv  [[attribute(1)]];\n"
            "};\n"
            "struct vs_out {\n"
            "    float4 pos [[position]];\n"
            "    float2 uv;\n"
            "};\n"
            "vertex vs_out vs_main(vs_in in [[stage_in]], constant float4x4 &mvp [[buffer(0)]]) {\n"
            "    vs_out out;\n"
            "    out.pos = mvp * float4(in.pos, 1.0);\n"
            "    out.uv = in.uv;\n"
            "    return out;\n"
            "}\n"
    },
    .fragment_func = {
        .entry  = "fs_main",
        .source = "#include <metal_stdlib>\n"
            "using namespace metal;\n"
            "fragment float4 fs_main(float2 uv [[stage_in]],\n"
            "                        texture2d<float> tex [[texture(0)]],\n"
            "                        sampler smp [[sampler(0)]]) {\n"
            "    return tex.sample(smp, uv);\n"
            "}\n"
    }
});

    tex_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = tex_shader,
        .layout = {
            .buffers[0].stride = sizeof(float)*5, // 3 floats for pos, 2 for uv
            .attrs = {
                [0] = { .format = SG_VERTEXFORMAT_FLOAT3, .offset = 0 },
                [1] = { .format = SG_VERTEXFORMAT_FLOAT2, .offset = sizeof(float)*3 },
            }
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .label = "tex-pipeline"
    });

  }

void draw_textured_quad(mat4x4 view, mat4x4 proj) {
    // Build a model matrix for the quad (here we just use identity or a small translation)
    mat4x4 model;
    mat4x4_identity(model);

    model[0][0] = 8.0f;
    model[1][1] = 8.0f;
    model[2][2] = 8.0f;
    mat4x4 mvp;
    mat4x4_mul(mvp, view, model);
    mat4x4_mul(mvp, proj, mvp);

    // Use the same uniform structure as your cube (assuming a struct with a 4x4 float matrix)
    vs_params_t vs_params;
    memcpy(vs_params.mvp, mvp, sizeof(mvp));

    sg_apply_pipeline(tex_pip);
    sg_apply_bindings(&quad_bind);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_draw(0, 6, 1);
}


// vvvv move these functions out

void load_texture(void) {
    // Load image from disk ("test.png" can be any image file)
    int img_width, img_height, img_channels;
unsigned char* img_data = stbi_load("assets/kermit.jpg", &img_width, &img_height, &img_channels, 4);
if (!img_data) {
    fprintf(stderr, "Failed to load image 'test.png'!\n");
    exit(1);
} else {
    printf("Loaded image: %dx%d, channels: %d\n", img_width, img_height, img_channels);
}
    // Create a Sokol texture from the loaded pixel data
my_img = sg_make_image(&(sg_image_desc){
    .width = img_width,
    .height = img_height,
    .pixel_format = SG_PIXELFORMAT_RGBA8,
    .data.subimage[0][0] = { .ptr = img_data, .size = (size_t)(img_width * img_height * 4) }
});
if (my_img.id == SG_INVALID_ID) {
    fprintf(stderr, "ERROR: Texture creation failed!\n");
}
stbi_image_free(img_data);
}


void reset_movement_keys(CameraControls* control) {
    control->key_w = false;
    control->key_a = false;
    control->key_s = false;
    control->key_d = false;
}

// ^^^^ end: move these functions out

void init(void) {
    sg_setup(&(sg_desc){
            .environment = sglue_environment(),
            .logger.func = slog_func,
        });
    
    snk_setup(&(snk_desc_t){0});
    nk_style_hide_cursor(snk_new_frame());

    memset(&world, 0, sizeof(World));

    world.in_edit_mode = true; // start in edit mode.

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

    /* create_cube(&world, 100.0f, 100.0f, 100.0f); */
    /* create_cube(&world, 5.5f, 0.0f, 0.0f); */
    /* create_cube(&world, 1.0f, 10.0f, 6.0f); */
    /* create_cube(&world, 12.3f, 17.0f, 9.0f); */
    /* create_cube(&world, 1.0f, 3.0f, 8.0f); */

    printf("before create cube_new\n");

    create_cube_new(&world, 5.0f, 5.0f, 5.0f);
    
    printf("after create cube_new\n");

    create_grid(&world);

    create_camera(&world, 72.0f, 23.0f, -5.0f, 8.0f, 8.0f, 8.0f, "editor camera default");
    create_camera(&world, 130.0f, 20.50f, -14.0f, 17.0f, 0.0f, 11.5f, "Wide Shot");
    create_camera(&world, 80.0f, -7.0f, -180.0f, 14.0f, 0.0f, 44.75f, "under");
    create_camera(&world, 43.0f, 3.95f, -343.0f, 2.0f, 0.0f, -21.0f, "close 2 Shot");
    create_camera(&world, 2.0f, 55.93f, -1.69f, -1.38f, 0.0f, 5.84f, "kermit");

    world.camera = world.cameras[EDITOR_CAMERA_INDEX];

    load_texture();
    create_textured_quad();
    create_tex_pipeline();
    
    free(vs_source);
    free(fs_source);
    free(grid_vs_source);
    free(grid_fs_source);
}

void input(const sapp_event* ev) {
    
    switch (ev->type) {
    case SAPP_EVENTTYPE_KEY_DOWN:
        if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
            if (!world.in_edit_mode) {
                world.camera = world.cameras[EDITOR_CAMERA_INDEX];
                world.in_edit_mode = true;
            }
        }
        else if (ev->key_code == SAPP_KEYCODE_W) world.control.key_w = true;
        else if (ev->key_code == SAPP_KEYCODE_A) world.control.key_a = true;
        else if (ev->key_code == SAPP_KEYCODE_S) world.control.key_s = true;
        else if (ev->key_code == SAPP_KEYCODE_D) world.control.key_d = true;
        break;

    case SAPP_EVENTTYPE_KEY_UP:
        if (ev->key_code == SAPP_KEYCODE_W) world.control.key_w = false;
        else if (ev->key_code == SAPP_KEYCODE_A) world.control.key_a = false;
        else if (ev->key_code == SAPP_KEYCODE_S) world.control.key_s = false;
        else if (ev->key_code == SAPP_KEYCODE_D) world.control.key_d = false;
        break;

    case SAPP_EVENTTYPE_MOUSE_DOWN:
        if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            world.control.mouse_down = true;
            world.control.last_mouse_x = ev->mouse_x;
            world.control.last_mouse_y = ev->mouse_y;
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_UP:
        if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            world.control.mouse_down = false;
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        world.camera.distance -= ev->scroll_y;
        world.camera.distance = clamp(2.0f, world.camera.distance, 20.0f);
        break;

    case SAPP_EVENTTYPE_MOUSE_MOVE:
        if (world.control.mouse_down) {
            float delta_x = ev->mouse_x - world.control.last_mouse_x;
            float delta_y = ev->mouse_y - world.control.last_mouse_y;

            world.camera.yaw += -delta_x * 0.3f;
            world.camera.pitch += delta_y * 0.3f;                
                
            world.camera.pitch = clamp(world.camera.pitch, -89.0f, 89.0f);

            world.control.last_mouse_x = ev->mouse_x;
            world.control.last_mouse_y = ev->mouse_y;
        }
        break;

    default:
        break;
    }
    // handle nuklear last
    if (snk_handle_event(ev)) {
        return;
    }
}

void frame(void) {
    const float w = sapp_widthf();
    const float h = sapp_heightf();

    float normalized_yaw = fmodf(world.camera.yaw, 360.0f);
    if (normalized_yaw < 0) normalized_yaw += 360.0f;

    float pitch_rad = TO_RAD(world.camera.pitch);
    float yaw_rad = TO_RAD(normalized_yaw);

    // WASD movement calculations
    float move_speed = 0.5f;
    vec3 forward = {
        sinf(yaw_rad),
        0,
        cosf(yaw_rad)
    };
    
    vec3 right = {
        cosf(yaw_rad),
        0,
        -sinf(yaw_rad)
    };

    // Update position based on key states
    if (world.control.key_w) {
        world.camera.position[0] += -forward[0] * move_speed;
        world.camera.position[2] += -forward[2] * move_speed;
    }
    if (world.control.key_s) {
        world.camera.position[0] -= -forward[0] * move_speed;
        world.camera.position[2] -= -forward[2] * move_speed;
    }
    if (world.control.key_a) {
        world.camera.position[0] -= right[0] * move_speed;
        world.camera.position[2] -= right[2] * move_speed;
    }
    if (world.control.key_d) {
        world.camera.position[0] += right[0] * move_speed;
        world.camera.position[2] += right[2] * move_speed;
    }

    // Calculate camera position
    float cam_x = world.camera.position[0] + world.camera.distance * cosf(pitch_rad) * sinf(yaw_rad);
    float cam_y = world.camera.position[1] + world.camera.distance * sinf(pitch_rad);
    float cam_z = world.camera.position[2] + world.camera.distance * cosf(pitch_rad) * cosf(yaw_rad);

    mat4x4 proj;
    mat4x4 view;
    mat4x4_perspective(proj, TO_RAD(60.0f), w/h, 0.1f, 500.0f);
    
    vec3 eye = {cam_x, cam_y, cam_z};
    vec3 center = {world.camera.position[0], world.camera.position[1], world.camera.position[2]};
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

    draw_textured_quad(view, proj);
    render_entities(&world, view, proj);
    
    nk_ctx = snk_new_frame();
    if (nk_begin(nk_ctx, "nukes", nk_rect(0, 0, 200, 400),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
    
        nk_layout_row_static(nk_ctx, 30, 80, 1);
        if (nk_button_label(nk_ctx, "save scene")) {
            printf("saving scene - TO BE IMPLEMENTED");
        }

        nk_layout_row_dynamic(nk_ctx, 20, 1);
        nk_label(nk_ctx, "Camera Position:", NK_TEXT_LEFT);
    
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Distance: %.2f", world.camera.distance);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);
    
        snprintf(buffer, sizeof(buffer), "Pitch: %.2f°", world.camera.pitch);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);
    
        snprintf(buffer, sizeof(buffer), "Yaw: %.2f°", world.camera.yaw);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);
    
        snprintf(buffer, sizeof(buffer), "X: %.2f", world.camera.position[0]);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);
    
        snprintf(buffer, sizeof(buffer), "Y: %.2f", world.camera.position[1]);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);
    
        snprintf(buffer, sizeof(buffer), "Z: %.2f", world.camera.position[2]);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);

        nk_layout_row_dynamic(nk_ctx, 20, 1);
        nk_label(nk_ctx, "Camera Switcher:", NK_TEXT_LEFT);

        // index 0 is reserved for the editor cam for now
        for (int i = 1; i < world.camera_count; i++) {
            snprintf(buffer, sizeof(buffer), "Camera %d: %s", i+1, world.cameras[i].name);
            if (nk_button_label(nk_ctx, buffer)) {
                reset_movement_keys(&world.control);
                if (world.in_edit_mode) {
                    world.cameras[EDITOR_CAMERA_INDEX] = world.camera;
                    world.in_edit_mode = false;
                }
                
                Camera* selected = &world.cameras[i];
                world.camera.position[0] = selected->position[0];
                world.camera.position[1] = selected->position[1];
                world.camera.position[2] = selected->position[2];
                world.camera.distance = selected->distance;
                world.camera.pitch = selected->pitch;
                world.camera.yaw = selected->yaw;
                world.in_edit_mode = false;
            }
        }

        if (world.in_edit_mode) {
            nk_label(nk_ctx, "<In Editor Mode>", NK_TEXT_CENTERED);
        }
    }
    snk_render(sapp_width(), sapp_height());
    nk_end(nk_ctx);

    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    nk_free(nk_ctx);
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
