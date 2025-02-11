#include "camera.h"

#define TO_RAD(deg) ((deg) * M_PI / 180.0f)

void update_camera_frame(World* world) {
    float move_speed = 0.5f;
    
    // Calculate normalized yaw first
    float normalized_yaw = fmodf(world->camera.yaw, 360.0f);
    if (normalized_yaw < 0) normalized_yaw += 360.0f;

    float pitch_rad = TO_RAD(world->camera.pitch);
    float yaw_rad = TO_RAD(normalized_yaw);

    // WASD movement calculations
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
    if (world->control.key_w) {
        world->camera.position[0] += -forward[0] * move_speed;
        world->camera.position[2] += -forward[2] * move_speed;
    }
    if (world->control.key_s) {
        world->camera.position[0] -= -forward[0] * move_speed;
        world->camera.position[2] -= -forward[2] * move_speed;
    }
    if (world->control.key_a) {
        world->camera.position[0] -= right[0] * move_speed;
        world->camera.position[2] -= right[2] * move_speed;
    }
    if (world->control.key_d) {
        world->camera.position[0] += right[0] * move_speed;
        world->camera.position[2] += right[2] * move_speed;
    }

    // Calculate final camera position
    world->camera.eye[0] = world->camera.position[0] + world->camera.distance * cosf(pitch_rad) * sinf(yaw_rad);
    world->camera.eye[1] = world->camera.position[1] + world->camera.distance * sinf(pitch_rad);
    world->camera.eye[2] = world->camera.position[2] + world->camera.distance * cosf(pitch_rad) * cosf(yaw_rad);
}

void create_camera(World* world,
                  float x, float y, float z,  // Exact camera position
                  float pitch, float yaw,      // Camera orientation
                  const char* name) {
    if (world->camera_count >= 16) {
        printf("Warning: Maximum number of cameras reached\n");
        return;
    }

    Camera* new_cam = &world->cameras[world->camera_count];
    
    // Store exact position
    new_cam->position[0] = x;
    new_cam->position[1] = y;
    new_cam->position[2] = z;
    new_cam->pitch = pitch;
    new_cam->yaw = yaw;
    
    new_cam->id = world->next_camera_id++;
    strncpy(new_cam->name, name, 31);
    new_cam->name[31] = '\0';

    world->camera_count++;
}
