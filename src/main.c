#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "linmath.h"
#include "utils_math.h"

#define SOKOL_METAL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "types.h"
#include "camera.h"
#include "ecs.h"
#include "gui.h"
#include "render.h"
#include "character.h"
#include "utils.h"

#include "../libs/sokol/sokol_debugtext.h"
#include "cJSON.h"

World world;

void cleanup(void);

void init(void)
{
    sg_setup(&(sg_desc){
            .environment = sglue_environment(),
            .logger.func = slog_func,
        });
    
    memset(&world, 0, sizeof(World));
    world.in_edit_mode = true; // start in edit mode
    world.show_grid = 0;
    world.quit = false;

    // todo
    /* char* script = read_text_file("api/generated/podcast_1739576207/script.json"); */
    /* if (!script) { */
    /*     printf("cant read script gg\n"); */
    /* } else { */
    /*     printf("hers the cript: %s\n", script); */
    /* } */
    /* cJSON* json = cJSON_Parse(script); */
    /* if (json == NULL) { */
    /*     const char *error_ptr = cJSON_GetErrorPtr(); */
    /*     if (error_ptr != NULL) { */
    /*         printf("Error parsing JSON: %s\n", error_ptr); */
    /*     }         */
    /* } else { */
    /*     parse_script(json); */
    /*     cJSON_Delete(json); */
    /*     free(script); */
    /* } */
    // end todo

    init_nuklear_gui(&world);

    render_init();

    create_and_set_grid(&world);
    
    /* create_img(&world, "assets/kermit.jpg", (vec3) {0.0f, -5.0f, 0.0f}, (vec3) {75.0f, 75.0f, 75.0f}); */
    /* create_img(&world, "assets/farm.jpg", (vec3) {-113.0f, 0.0f, 124.0f}, (vec3) {75.0f, 75.0f, 75.0f}); */

    create_and_add_camera(&world, 82.76f, 75.0f, -106.12f, -30.0f, -395.0f, "default camera");

    create_character_poscam(&world, "assets/buu2.jpeg", "buu_guy", "majin buu", (vec3) { 5.0f, 5.0f, 20.0f }, (vec3) { 4.22f, 4.0f, 35.76f }, (vec2){8.25f, -180.15f});
    create_character_poscam(&world, "assets/kermit.jpg", "kermit_da_frog", "Kermit the frog", (vec3) { 30.0f, 0.0f, 30.0f }, (vec3) { 30.0f, 4.0f, 46.12f }, (vec2) { -9.50f, -180.0f});

    // DEFAULT EDITOR CAM
    
    
    /* create_and_add_camera(&world, 80.0f, 8.0f, -55.23f, -7.23f, -402.8f, "kermit left"); */
    /* create_and_add_camera(&world, -101.0f, 8.0f, 57.32f, -5.13f, -361.86f, "farm"); */

    init_camera_visualization(&world);
    world.active_camera = world.cameras[EDITOR_CAMERA_INDEX];
}

void input(const sapp_event* ev)
{
    switch (ev->type) {
    case SAPP_EVENTTYPE_KEY_DOWN:
        if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
            if (!world.in_edit_mode) {
                world.active_camera = world.cameras[EDITOR_CAMERA_INDEX];
                world.in_edit_mode = true;
            }
        }
        else if (ev->key_code == SAPP_KEYCODE_W) world.control.key_w = true;
        else if (ev->key_code == SAPP_KEYCODE_A) world.control.key_a = true;
        else if (ev->key_code == SAPP_KEYCODE_S) world.control.key_s = true;
        else if (ev->key_code == SAPP_KEYCODE_D) world.control.key_d = true;
        else if (ev->key_code == SAPP_KEYCODE_SPACE) world.control.key_spc = true;
        else if (ev->key_code == SAPP_KEYCODE_LEFT_SHIFT) world.control.key_shift = true;
        break;

    case SAPP_EVENTTYPE_KEY_UP:
        if (ev->key_code == SAPP_KEYCODE_W) world.control.key_w = false;
        else if (ev->key_code == SAPP_KEYCODE_A) world.control.key_a = false;
        else if (ev->key_code == SAPP_KEYCODE_S) world.control.key_s = false;
        else if (ev->key_code == SAPP_KEYCODE_D) world.control.key_d = false;
        else if (ev->key_code == SAPP_KEYCODE_SPACE) world.control.key_spc = false;
        else if (ev->key_code == SAPP_KEYCODE_LEFT_SHIFT) world.control.key_shift = false;
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
        break;

    case SAPP_EVENTTYPE_MOUSE_MOVE:
        if (world.control.mouse_down) {
            float delta_x = ev->mouse_x - world.control.last_mouse_x;
            float delta_y = ev->mouse_y - world.control.last_mouse_y;

            // Increase sensitivity for more noticeable movement
            world.active_camera.yaw += -delta_x * 0.5f;
            world.active_camera.pitch += -delta_y * 0.5f;  // Negative for intuitive up/down looking
                
            // Clamp pitch to prevent camera flipping
            world.active_camera.pitch = clamp(world.active_camera.pitch, -89.0f, 89.0f);

            world.control.last_mouse_x = ev->mouse_x;
            world.control.last_mouse_y = ev->mouse_y;
        }
        break;

    default:
        break;
    }
    if (snk_handle_event(ev)) {
        return;
    }
}

void frame(void)
{
    if (world.quit) {
        cleanup();
        exit(1);
    }
    
    const float w = sapp_widthf();
    const float h = sapp_heightf();

    update_camera_frame(&world);

    mat4x4 proj;
    mat4x4_perspective(proj, TO_RAD(60.0f), w/h, 0.1f, 500.0f);

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

    //render_text(10.0f, 10.0f, "subtitles go here probably?!");

    if (world.show_grid) {
        render_grid(&world, world.active_camera.view, proj);
    }
    render_entities(&world, world.active_camera.view, proj);
    render_cameras(&world, world.active_camera.view, proj);
    draw_nuklear_gui(&world);

    sdtx_draw();
    sg_end_pass();
    sg_commit();
}

void cleanup(void)
{
    nk_free(world.ctx);
    snk_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[])
{
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
