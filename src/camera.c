#include "camera.h"

/* void update_camera_frame(World* world) { */
/*     float move_speed = 0.5f; */
/*     float pitch_rad = TO_RAD(world.camera.pitch); */
/*     float yaw_rad = TO_RAD(normalized_yaw); */
/*     float normalized_yaw = fmodf(world->camera.yaw, 360.0f); */
/*     if (normalized_yaw < 0) normalized_yaw += 360.0f; */

/*     // WASD movement calculations */
/*     world->camera.forward = { */
/*         sinf(yaw_rad), */
/*         0, */
/*         cosf(yaw_rad) */
/*     }; */
    
/*     world->camera.right = { */
/*         cosf(yaw_rad), */
/*         0, */
/*         -sinf(yaw_rad) */
/*     };     */
/* } */

/* void create_camera(World* world, */
/*                    float distance, */
/*                    float pitch, */
/*                    float yaw, */
/*                    float x, */
/*                    float y, */
/*                    float z, */
/*                    const char* name) { */
/*     if (world->camera_count >= 16) { */
/*         printf("Warning: Maximum number of cameras reached\n"); */
/*         return; */
/*     } */

/*     Camera* new_cam = &world->cameras[world->camera_count]; */
    
/*     // Initialize camera properties */
/*     new_cam->distance = distance; */
/*     new_cam->pitch = pitch; */
/*     new_cam->yaw = yaw; */
/*     new_cam->position[0] = x; */
/*     new_cam->position[1] = y; */
/*     new_cam->position[2] = z; */
/*     new_cam->id = world->next_camera_id++; */
/*     strncpy(new_cam->name, name, 31); */
/*     new_cam->name[31] = '\0';  // Ensure null termination */
    
/*     // Initialize default values */
/*     new_cam->center[0] = 0.0f;  // Looking at origin */
/*     new_cam->center[1] = 0.0f; */
/*     new_cam->center[2] = 0.0f; */
/*     new_cam->up[0] = 0.0f;     // Up vector */
/*     new_cam->up[1] = 1.0f; */
/*     new_cam->up[2] = 0.0f; */

/*     world->camera_count++; */
/* } */

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
