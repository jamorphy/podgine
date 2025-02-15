#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "linmath.h"
#include "cJSON.h"

#define SOKOL_METAL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "sokol_debugtext.h"

#include "types.h"
#include "camera.h"
#include "ecs.h"
#include "gui.h"
#include "render.h"
#include "character.h"
#include "scene.h"
#include "audio.h"
#include "utils.h"
#include "utils_math.h"

World world;

void cleanup(void);

void play_next_line(World* world) {
    if (!world->is_playing_audio && 
        world->current_line < world->script->line_count) {
        
        // Start playing the current line
        audio_play_file(world->script->lines[world->current_line].audio_file);
        world->is_playing_audio = true;
    }
    
    // Check if current audio finished
    if (world->is_playing_audio) {
        if (!audio_is_playing()) {
            // Audio finished, move to next line
            world->is_playing_audio = false;
            world->current_line++;
        }
    }
}

void init(void)
{
    sg_setup(&(sg_desc){
            .environment = sglue_environment(),
            .logger.func = slog_func,
        });
    
    memset(&world, 0, sizeof(World));
    world.in_edit_mode = true;
    world.show_grid = 0;
    world.quit = false;

    audio_init();

    // TODO: 
    char* script_text = read_text_file("api/generated/podcast_1739601442/script.json");
    if (!script_text) {
        printf("cant read script gg\n");
    } else {
        printf("hers the cript: %s\n", script_text);
    }
    cJSON* json = cJSON_Parse(script_text);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error parsing JSON: %s\n", error_ptr);
        }
    } else {
        // Debug: Check if we can access dialogue array
        cJSON* dialogue = cJSON_GetObjectItem(json, "dialogue");
        if (dialogue) {
            int count = cJSON_GetArraySize(dialogue);
            printf("Found dialogue array with %d items\n", count);
            
            // Debug: Try to access first item
            cJSON* first_item = cJSON_GetArrayItem(dialogue, 0);
            if (first_item) {
                cJSON* char_item = cJSON_GetObjectItem(first_item, "character");
                if (char_item) {
                    printf("First character: %s\n", char_item->valuestring);
                }
            }
        }

        parse_script(&world, json);
        cJSON_Delete(json);
        free(script_text);
    }
    // end todo

    if (world.script) {
        printf("Accessing %d lines:\n", world.script->line_count);
        for (int i = 0; i < world.script->line_count; i++) {
            Line* currentLine = &world.script->lines[i];
            printf("[%d] %s: %s %s\n", 
                i,
                currentLine->character,
                currentLine->text,
                currentLine->audio_file);
        }
    }

    init_nuklear_gui(&world);
    render_init();
    // TODO: this create_ api is terrible
    create_and_set_grid(&world);    
    create_and_add_camera(&world, 82.76f, 75.0f, -106.12f, -30.0f, -395.0f, "default camera");
    create_character_poscam(&world, "assets/buu2.jpeg", "buu_guy", "majin buu", (vec3) { 5.0f, 5.0f, 20.0f }, (vec3) { 4.22f, 4.0f, 35.76f }, (vec2){8.25f, -180.15f});
    create_character_poscam(&world, "assets/kermit.jpg", "kermit_da_frog", "Kermit the frog", (vec3) { 30.0f, 0.0f, 30.0f }, (vec3) { 30.0f, 4.0f, 46.12f }, (vec2) { -9.50f, -180.0f});
    init_camera_renderable(&world);
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
            world.active_camera.pitch += -delta_y * 0.5f;
                
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

    play_next_line(&world);

    sdtx_draw();
    sg_end_pass();
    sg_commit();
}

void cleanup(void)
{
    audio_shutdown();
    clear_scene(&world);
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
        .window_title = "podgine",
        .icon.sokol_default = false,
        .swap_interval = 1
    };
}
