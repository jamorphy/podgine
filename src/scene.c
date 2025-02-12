#include "../libs/cJSON/cJSON.h"

#include <stdlib.h>
#include "scene.h"
#include "ecs.h"
#include "constants.h"
#include "camera.h"

void clear_scene(World* world) {
    Camera editor_camera = world->cameras[0];

    for (uint32_t i = 0; i < world->renderable_count; i++) {
        Renderable* renderable = &world->renderables[i];
        if (renderable->material) {
            // Destroy shader and pipeline
            if (renderable->material->shader.id != 0) {
                sg_destroy_shader(renderable->material->shader);
            }
            if (renderable->material->pipeline.id != 0) {
                sg_destroy_pipeline(renderable->material->pipeline);
            }
        }
        if (renderable->mesh) {
            // Destroy buffers
            if (renderable->mesh->vertex_buffer.id != 0) {
                sg_destroy_buffer(renderable->mesh->vertex_buffer);
            }
            if (renderable->mesh->index_buffer.id != 0) {
                sg_destroy_buffer(renderable->mesh->index_buffer);
            }
        }
    }

    world->renderable_count = 0;

    // Destroy all entities
    while (world->entity_count > 0) {
        uint32_t current_index = world->entity_count - 1;
        Entity* entity_to_destroy = &world->entities[current_index];
        
        destroy_entity(world, entity_to_destroy);
    }
    
    world->next_entity_id = 1;
    
    world->camera_count = 1;
    world->next_camera_id = 1;
    
    // Explicitly reset non-editor cameras
    for (int i = 1; i < MAX_CAMERAS; i++) {
        Camera* cam = &world->cameras[i];
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
}

bool load_scene(World* world, const char* filename) {
    // Build full path
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "scenes/%s", filename);
    
    // Open file
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open scene file: %s\n", filepath);
        return false;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size_long = ftell(file);
    if (file_size_long < 0) {
        fprintf(stderr, "Error getting file size\n");
        fclose(file);
        return false;
    }
    size_t file_size = (size_t)file_size_long;
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer and read file
    char* json_buffer = (char*)malloc(file_size + 1);
    if (!json_buffer) {
        fprintf(stderr, "Failed to allocate memory for scene file\n");
        fclose(file);
        return false;
    }
    
    size_t read_size = fread(json_buffer, 1, file_size, file);
    fclose(file);
    
    if (read_size != file_size) {
        fprintf(stderr, "Failed to read entire scene file\n");
        free(json_buffer);
        return false;
    }
    
    json_buffer[file_size] = '\0';  // Null terminate the string
    
    // Parse JSON
    cJSON* root = cJSON_Parse(json_buffer);
    free(json_buffer);  // We can free this now as cJSON has its own copy
    
    if (!root) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            fprintf(stderr, "JSON Parse Error before: %s\n", error_ptr);
        }
        return false;
    }
    
    // Version check
    cJSON* version = cJSON_GetObjectItem(root, "scene_version");
    if (!version || !cJSON_IsNumber(version) || version->valueint != 1) {
        fprintf(stderr, "Invalid or unsupported scene version\n");
        cJSON_Delete(root);
        return false;
    }
    
    // Clear existing scene
    clear_scene(world);
    
    // TODO: Load entities and cameras
    cJSON* entities = cJSON_GetObjectItem(root, "entities");
    if (!entities || !cJSON_IsArray(entities)) {
        fprintf(stderr, "Scene file missing entities array or invalid format\n");
        cJSON_Delete(root);
        return false;
    }

    int entity_count = cJSON_GetArraySize(entities);
    for (int i = 0; i < entity_count; i++) {
        cJSON* entity_json = cJSON_GetArrayItem(entities, i);
        cJSON* components = cJSON_GetObjectItem(entity_json, "components");
        if (!components) continue;

        // Get transform data
        cJSON* transform = cJSON_GetObjectItem(components, "transform");
        if (!transform) continue;

        // Parse position
        vec3 position = {0, 0, 0};
        cJSON* pos_array = cJSON_GetObjectItem(transform, "position");
        if (pos_array && cJSON_IsArray(pos_array)) {
            position[0] = (float)cJSON_GetArrayItem(pos_array, 0)->valuedouble;
            position[1] = (float)cJSON_GetArrayItem(pos_array, 1)->valuedouble;
            position[2] = (float)cJSON_GetArrayItem(pos_array, 2)->valuedouble;
        }

        // Parse scale
        vec3 scale = {1, 1, 1};
        cJSON* scale_array = cJSON_GetObjectItem(transform, "scale");
        if (scale_array && cJSON_IsArray(scale_array)) {
            scale[0] = (float)cJSON_GetArrayItem(scale_array, 0)->valuedouble;
            scale[1] = (float)cJSON_GetArrayItem(scale_array, 1)->valuedouble;
            scale[2] = (float)cJSON_GetArrayItem(scale_array, 2)->valuedouble;
        }

        // Get mesh type
        cJSON* mesh = cJSON_GetObjectItem(components, "mesh");
        if (!mesh) continue;

        cJSON* mesh_type = cJSON_GetObjectItem(mesh, "type");
        if (!mesh_type || !cJSON_IsString(mesh_type)) continue;

        Entity* entity = NULL;
        if (strcmp(mesh_type->valuestring, "cube") == 0) {
            entity = create_cube(world, position, scale);
        }
        else if (strcmp(mesh_type->valuestring, "image") == 0) {
            // For images, we need to get the image path
            cJSON* image_path = cJSON_GetObjectItem(mesh, "image_path");
            if (!image_path || !cJSON_IsString(image_path)) {
                fprintf(stderr, "Image entity missing image_path\n");
                continue;
            }
            entity = create_img(world, image_path->valuestring, position, scale);
        }

        if (!entity) {
            fprintf(stderr, "Failed to create entity of type: %s\n", mesh_type->valuestring);
            cJSON_Delete(root);
            return false;
        }
    }

    cJSON* cameras = cJSON_GetObjectItem(root, "cameras");
    if (!cameras || !cJSON_IsArray(cameras)) {
        fprintf(stderr, "Scene file missing cameras array or invalid format\n");
        cJSON_Delete(root);
        return false;
    }

    int camera_count = cJSON_GetArraySize(cameras);
    for (int i = 0; i < camera_count; i++) {
        cJSON* camera = cJSON_GetArrayItem(cameras, i);
        
        // Get camera name
        cJSON* name = cJSON_GetObjectItem(camera, "name");
        if (!name || !cJSON_IsString(name)) continue;
        
        // Get position
        cJSON* pos_array = cJSON_GetObjectItem(camera, "position");
        float x = 0, y = 0, z = 0;
        if (pos_array && cJSON_IsArray(pos_array)) {
            x = (float)cJSON_GetArrayItem(pos_array, 0)->valuedouble;
            y = (float)cJSON_GetArrayItem(pos_array, 1)->valuedouble;
            z = (float)cJSON_GetArrayItem(pos_array, 2)->valuedouble;
        }
        
        // Get orientation
        float pitch = 0, yaw = 0;
        cJSON* pitch_json = cJSON_GetObjectItem(camera, "pitch");
        cJSON* yaw_json = cJSON_GetObjectItem(camera, "yaw");
        
        if (pitch_json && cJSON_IsNumber(pitch_json)) {
            pitch = (float)pitch_json->valuedouble;
        }
        if (yaw_json && cJSON_IsNumber(yaw_json)) {
            yaw = (float)yaw_json->valuedouble;
        }
        
        // Create the camera
        create_camera(world, x, y, z, pitch, yaw, name->valuestring);
    }
    
    
    // Cleanup
    cJSON_Delete(root);
    
    return true;
}
