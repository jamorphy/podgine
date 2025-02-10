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

#include "types.h"
#include "camera.h"
#include "ecs.h"
#include "gui.h"

World world;

void init(void)
{
    sg_setup(&(sg_desc){
            .environment = sglue_environment(),
            .logger.func = slog_func,
        });
    
    memset(&world, 0, sizeof(World));
    world.in_edit_mode = true; // start in edit mode

    init_nuklear_gui(&world);

    create_cube(&world, (vec3) { 5.0f, 5.0f, 5.0f },    (vec3) { 2.0f, 2.0f, 2.0f });
    create_cube(&world, (vec3) { -15.0f, 10.0f, 9.0f }, (vec3) { 3.0f, 3.0f, 4.0f });

    create_img(&world, "assets/kermit.jpg", (vec3) {0.0f, -5.0f, 0.0f}, (vec3) {75.0f, 75.0f, 75.0f});
    create_img(&world, "assets/farm.jpg", (vec3) {-113.0f, 0.0f, 124.0f}, (vec3) {75.0f, 75.0f, 75.0f});
    
    create_camera(&world, 72.0f, 23.0f, -5.0f, 8.0f, 8.0f, 8.0f, "editor camera default");
    create_camera(&world, 130.0f, 20.50f, -14.0f, 17.0f, 0.0f, 11.5f, "Wide Shot");
    create_camera(&world, 80.0f, -7.0f, -180.0f, 14.0f, 0.0f, 44.75f, "under");
    create_camera(&world, 43.0f, 3.95f, -343.0f, 2.0f, 0.0f, -21.0f, "close 2 Shot");
    create_camera(&world, 2.0f, 55.93f, -1.69f, -1.38f, 0.0f, 5.84f, "kermit");

    world.camera = world.cameras[EDITOR_CAMERA_INDEX];
}

void input(const sapp_event* ev)
{
    
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
    if (snk_handle_event(ev)) {
        return;
    }
}

void frame(void)
{
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

    render_entities(&world, view, proj);
    draw_nuklear_gui(&world);

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
