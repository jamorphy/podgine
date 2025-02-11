#include "gui.h"

#include <stdlib.h>

void reset_movement_keys(CameraControls* control) {
    control->key_w = false;
    control->key_a = false;
    control->key_s = false;
    control->key_d = false;
}

void init_nuklear_gui(World* world) {
    world->ctx = (struct nk_context*)malloc(sizeof(struct nk_context));
    snk_setup(&(snk_desc_t){0});
    nk_style_hide_cursor(snk_new_frame());
}

void draw_nuklear_gui(World* world) {
    
    struct nk_context* nk_ctx = world->ctx;

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
    
        snprintf(buffer, sizeof(buffer), "Pitch: %.2f°", world->camera.pitch);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);
    
        snprintf(buffer, sizeof(buffer), "Yaw: %.2f°", world->camera.yaw);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);
    
        snprintf(buffer, sizeof(buffer), "X: %.2f", world->camera.position[0]);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);
    
        snprintf(buffer, sizeof(buffer), "Y: %.2f", world->camera.position[1]);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);
    
        snprintf(buffer, sizeof(buffer), "Z: %.2f", world->camera.position[2]);
        nk_label(nk_ctx, buffer, NK_TEXT_LEFT);

        nk_layout_row_dynamic(nk_ctx, 20, 1);
        nk_label(nk_ctx, "Camera Switcher:", NK_TEXT_LEFT);

        // index 0 is reserved for the editor cam for now
        for (int i = 1; i < world->camera_count; i++) {
            snprintf(buffer, sizeof(buffer), "Camera %d: %s", i+1, world->cameras[i].name);
            if (nk_button_label(nk_ctx, buffer)) {
                reset_movement_keys(&world->control);
                if (world->in_edit_mode) {
                    world->cameras[0] = world->camera;
                    world->in_edit_mode = false;
                }
                
                Camera* selected = &world->cameras[i];
                world->camera.position[0] = selected->position[0];
                world->camera.position[1] = selected->position[1];
                world->camera.position[2] = selected->position[2];
                world->camera.distance = selected->distance;
                world->camera.pitch = selected->pitch;
                world->camera.yaw = selected->yaw;
                world->in_edit_mode = false;
            }
        }

        if (world->in_edit_mode) {
            nk_layout_row_static(nk_ctx, 30, 180, 1);  // Adjust width (180) as needed
            if (nk_button_label(nk_ctx, "Create Camera Here")) {
                create_camera_at_current_position(world);
            }
            nk_layout_row_dynamic(nk_ctx, 20, 1);
            nk_label(nk_ctx, "<In Editor Mode>", NK_TEXT_CENTERED);
        }
    }
    snk_render(sapp_width(), sapp_height());
    nk_end(nk_ctx);
}
