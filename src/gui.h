#pragma once
#include "state.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_COMMAND_USERDATA
#include "nuklear.h"

struct state;

void draw_nuklear_gui(struct state *state) {
    struct nk_context *ctx = state->nk_ctx;
    if (nk_begin(ctx, "Camera", nk_rect(0, 0, 200, 125),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_labelf(ctx, NK_TEXT_LEFT, "yolodistance: %.2f", state->camera_distance);
        nk_labelf(ctx, NK_TEXT_LEFT, "pitch: %.2f", state->camera_pitch);
        nk_labelf(ctx, NK_TEXT_LEFT, "yaw: %.2f", state->camera_yaw);
    }
    nk_end(ctx);
}
