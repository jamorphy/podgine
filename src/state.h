#pragma once

struct state {
    float rx, ry;
    sg_pipeline pip;
    sg_bindings bind;
    sg_pipeline grid_pip;
    sg_bindings grid_bind;
    float camera_distance;
    float camera_pitch;
    float camera_yaw;
    bool mouse_down;
    float last_mouse_x;
    float last_mouse_y;
    bool cube_selected;
    bool dragging;
    struct nk_context *nk_ctx;
    sg_pipeline gizmo_pip;
    sg_bindings gizmo_bind;
    vec3 cube_position;
};
