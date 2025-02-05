#define SOKOL_METAL
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"

// State struct
static struct {
    float rx, ry;
    sg_pipeline pip;
    sg_bindings bind;
    // Add these for orbit controls:
    float camera_distance;
    float camera_pitch;
    float camera_yaw;
    bool mouse_down;
    float last_mouse_x;
    float last_mouse_y;
} state;


typedef struct {
    hmm_mat4 mvp;
} vs_params_t;

// Shader sources
static const char* vs_src =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct params_t {\n"
    "  float4x4 mvp;\n"
    "};\n"
    "struct vs_in {\n"
    "  float4 position [[attribute(0)]];\n"
    "  float4 color [[attribute(1)]];\n"
    "};\n"
    "struct vs_out {\n"
    "  float4 pos [[position]];\n"
    "  float4 color;\n"
    "};\n"
    "vertex vs_out _main(vs_in in [[stage_in]], constant params_t& params [[buffer(0)]]) {\n"
    "  vs_out out;\n"
    "  out.pos = params.mvp * in.position;\n"
    "  out.color = in.color;\n"
    "  return out;\n"
    "}\n";

static const char* fs_src =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "fragment float4 _main(float4 color [[stage_in]]) {\n"
    "  return color;\n"
    "}\n";

void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
    });

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

    /* create shader */
    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0,
        },
        .vertex_func = {
            .source = vs_src,
            .entry = "_main"
        },
        .fragment_func = {
            .source = fs_src,
            .entry = "_main"
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
            // Zoom in/out with mouse wheel
            state.camera_distance -= ev->scroll_y;
            // Clamp distance to reasonable values
            if (state.camera_distance < 2.0f) state.camera_distance = 2.0f;
            if (state.camera_distance > 20.0f) state.camera_distance = 20.0f;
        } break;

        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            if (state.mouse_down) {
                float delta_x = ev->mouse_x - state.last_mouse_x;
                float delta_y = ev->mouse_y - state.last_mouse_y;
                
                state.camera_yaw += -delta_x * 0.3f;
                state.camera_pitch += delta_y * 0.3f;
                
                // Clamp pitch to prevent camera flipping
                if (state.camera_pitch > 89.0f) state.camera_pitch = 89.0f;
                if (state.camera_pitch < -89.0f) state.camera_pitch = -89.0f;
                
                state.last_mouse_x = ev->mouse_x;
                state.last_mouse_y = ev->mouse_y;
            }
        } break;

        default: break;
    }
}

void frame(void) {
    const float w = sapp_widthf();
    const float h = sapp_heightf();
    //const float t = (float)(sapp_frame_duration() * 60.0);

    // Convert orbit angles to camera position
    float pitch_rad = HMM_ToRadians(state.camera_pitch);
    float yaw_rad = HMM_ToRadians(state.camera_yaw);

    // Calculate camera position using spherical coordinates
    float cam_x = state.camera_distance * HMM_CosF(pitch_rad) * HMM_SinF(yaw_rad);
    float cam_y = state.camera_distance * HMM_SinF(pitch_rad);
    float cam_z = state.camera_distance * HMM_CosF(pitch_rad) * HMM_CosF(yaw_rad);

    hmm_mat4 proj = HMM_Perspective(60.0f, w/h, 0.01f, 100.0f);
    hmm_mat4 view = HMM_LookAt(
        HMM_Vec3(cam_x, cam_y, cam_z),  // camera position
        HMM_Vec3(0.0f, 0.0f, 0.0f),     // look at center
        HMM_Vec3(0.0f, 1.0f, 0.0f)      // up vector
    );

    hmm_mat4 model = HMM_Mat4d(1.0f); // Identity matrix, no rotation needed
    vs_params_t vs_params;
    vs_params.mvp = HMM_MultiplyMat4(HMM_MultiplyMat4(proj, view), model);

    sg_begin_pass(&(sg_pass){
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.25f, 0.5f, 0.75f, 1.0f }
            },
        },
        .swapchain = sglue_swapchain()
    });
    
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_draw(0, 36, 1);
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = input,
        .width = 800,
        .height = 600,
        .sample_count = 4,
        .window_title = "Cube",
        .icon.sokol_default = true,
    };
}
