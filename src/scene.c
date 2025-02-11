#include "scene.h"
#include "ecs.h"
#include "constants.h"

void clear_scene(World* world) {
    printf("Starting scene clear...\n");
    
    // Store editor camera temporarily
    Camera editor_camera = world->cameras[0];
    printf("Stored editor camera (id: %d)\n", editor_camera.id);
    
    // Log initial state
    printf("Initial entity count: %d\n", world->entity_count);
    printf("Initial camera count: %d\n", world->camera_count);
    
    // Destroy all entities
    while (world->entity_count > 0) {
        uint32_t current_index = world->entity_count - 1;
        Entity* entity_to_destroy = &world->entities[current_index];
        printf("Destroying entity at index %d\n", current_index);
        
        if (entity_to_destroy->renderable->mesh) {
            printf("- Has mesh\n");
        }
        if (entity_to_destroy->renderable) {
            printf("- Has renderable\n");
            if (entity_to_destroy->renderable->material) {
                printf("- Has material\n");
            }
        }
        
        destroy_entity(world, entity_to_destroy);
        printf("Entity destroyed. New entity count: %d\n", world->entity_count);
    }
    
    // Reset entity-related counters
    world->next_entity_id = 1;
    
    // Reset camera-related counters, preserving editor camera
    printf("Resetting cameras (keeping editor camera)...\n");
    world->camera_count = 1;
    world->next_camera_id = 1;
    
    // Explicitly reset non-editor cameras
    for (int i = 1; i < MAX_CAMERAS; i++) {
        Camera* cam = &world->cameras[i];
        printf("Resetting camera at index %d\n", i);
        cam->id = 0;
        // TODO: make this a vec3
        cam->position[0] = 0.0;
        cam->position[1] = 0.0;
        cam->position[2] = 0.0;
        cam->pitch = 0;
        cam->yaw = 0;
        memset(cam->name, 0, sizeof(cam->name));
        // Reset any other camera properties to their default values
    }
    
    // Restore editor camera
    world->cameras[0] = editor_camera;
    world->camera = editor_camera;
    
    printf("Scene clear completed.\n");
    printf("Final entity count: %d\n", world->entity_count);
    printf("Final camera count: %d\n", world->camera_count);
}
