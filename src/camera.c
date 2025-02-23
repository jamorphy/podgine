#include "camera.h"
#include "log.h"
#include "constants.h"
#include "ecs.h"
#include "character.h"
#include "utils.h"

#define TO_RAD(deg) ((deg) * M_PI / 180.0f)

// TODO: this fn is just honestly bad
void update_camera_frame(World* world)
{
    float move_speed = 0.5f;

    float pitch_rad = TO_RAD(world->active_camera.pitch);
    float yaw_rad = TO_RAD(world->active_camera.yaw);

    // Calculate forward and right vectors for movement
    vec3 forward = {
        cosf(pitch_rad) * sinf(yaw_rad),   // x
        0,                                  // y (keeping movement horizontal)
        cosf(pitch_rad) * cosf(yaw_rad)    // z
    };

    vec3 right = {
        cosf(yaw_rad),    // x
        0,                // y
        -sinf(yaw_rad)    // z
    };

    // Update position based on key states
    if (world->control.key_w) {
        world->active_camera.position[0] += forward[0] * move_speed;
        world->active_camera.position[2] += forward[2] * move_speed;
    }
    if (world->control.key_s) {
        world->active_camera.position[0] -= forward[0] * move_speed;
        world->active_camera.position[2] -= forward[2] * move_speed;
    }
    if (world->control.key_a) {
        world->active_camera.position[0] -= -right[0] * move_speed;
        world->active_camera.position[2] -= -right[2] * move_speed;
    }
    if (world->control.key_d) {
        world->active_camera.position[0] += -right[0] * move_speed;
        world->active_camera.position[2] += -right[2] * move_speed;
    }
    if (world->control.key_spc) {
        world->active_camera.position[1] += move_speed;
    }
    if (world->control.key_shift) {
        world->active_camera.position[1] -= move_speed;
    }

    // Calculate target for view matrix
    vec3 target;
    target[0] = world->active_camera.position[0] + cosf(pitch_rad) * sinf(yaw_rad);
    target[1] = world->active_camera.position[1] + sinf(pitch_rad);
    target[2] = world->active_camera.position[2] + cosf(pitch_rad) * cosf(yaw_rad);

    // Update view matrix
    mat4x4_look_at(world->active_camera.view,
                   world->active_camera.position,
                   target,
                   (vec3){0.0f, 1.0f, 0.0f});
}

Camera* get_next_camera(World* world)
{
    if (world->camera_count >= MAX_CAMERAS) {
        LOG_WARN("Exceeded MAX_CAMERAS");
        return NULL;
    }
    
    Camera* cam = &world->cameras[world->camera_count];
    cam->id = world->next_camera_id++;
    world->camera_count++;
    return cam;
}

Camera* create_camera(World* world,
                      float x, float y, float z,
                      float pitch, float yaw)
{    
    Camera* camera = get_next_camera(world);
    camera->position[0] = x;
    camera->position[1] = y;
    camera->position[2] = z;
    camera->pitch = pitch;
    camera->yaw = yaw;    
    return camera;
}

void create_and_add_camera(World* world,
                           float x, float y, float z,
                           float pitch, float yaw,
                           const char* name)
{

    Camera* new_cam = get_next_camera(world);

    new_cam->position[0] = x;
    new_cam->position[1] = y;
    new_cam->position[2] = z;
    new_cam->pitch = pitch;
    new_cam->yaw = yaw;

    strncpy(new_cam->name, name, 31);
    new_cam->name[31] = '\0';
}

void create_camera_at_current_position(World* world)
{    

    // Generate a default name for the new camera
    char camera_name[32];
    snprintf(camera_name, sizeof(camera_name), "Camera %d", world->camera_count);

    // Create a new camera at the current editor camera position
    Camera current = world->active_camera;
    create_and_add_camera(world,
                 current.position[0],
                 current.position[1],
                 current.position[2],
                 current.pitch,
                 current.yaw,
                 camera_name);
}


void switch_to_character_camera(World *world, const char* character_id)
{
    Character* character = get_character(world, character_id);
    if (character == NULL) {
        LOG_FATAL("Failed to get character with id: %s", character_id);
    }
    world->active_camera.position[0] = character->cam->position[0];
    world->active_camera.position[1] = character->cam->position[1];
    world->active_camera.position[2] = character->cam->position[2];

    world->active_camera.pitch = character->cam->pitch;
    world->active_camera.yaw = character->cam->yaw;
}

void init_camera_renderable(World* world)
{
    sg_image texture = create_image_texture("assets/camera.jpg");

    if (world->renderable_count >= 1000) {
        LOG_FATAL("Exceeded MAX_RENDERABLES");
    }

    // Set up mesh and material
    world->camera_renderable.mesh = create_quad_mesh(texture);
    world->camera_renderable.material = create_textured_material();
}
