#include "camera.h"

void create_camera(World* world,
                   float distance,
                   float pitch,
                   float yaw,
                   float x,
                   float y,
                   float z,
                   const char* name) {
    if (world->camera_count >= 16) {
        printf("Warning: Maximum number of cameras reached\n");
        return;
    }

    Camera* new_cam = &world->cameras[world->camera_count];
    
    // Initialize camera properties
    new_cam->distance = distance;
    new_cam->pitch = pitch;
    new_cam->yaw = yaw;
    new_cam->position[0] = x;
    new_cam->position[1] = y;
    new_cam->position[2] = z;
    new_cam->id = world->next_camera_id++;
    strncpy(new_cam->name, name, 31);
    new_cam->name[31] = '\0';  // Ensure null termination
    
    // Initialize default values
    new_cam->center[0] = 0.0f;  // Looking at origin
    new_cam->center[1] = 0.0f;
    new_cam->center[2] = 0.0f;
    new_cam->up[0] = 0.0f;     // Up vector
    new_cam->up[1] = 1.0f;
    new_cam->up[2] = 0.0f;

    world->camera_count++;
}
