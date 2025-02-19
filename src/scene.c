#include <stdlib.h>
#include <stdbool.h>
#include "../libs/cJSON/cJSON.h"

#include "log.h"
#include "scene.h"
#include "ecs.h"
#include "constants.h"
#include "camera.h"
#include "character.h"

void clear_scene(World* world)
{
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
        cam->position[0] = 0.0;
        cam->position[1] = 0.0;
        cam->position[2] = 0.0;
        cam->pitch = 0;
        cam->yaw = 0;
        memset(cam->name, 0, sizeof(cam->name));
    }

    // Restore editor camera
    world->cameras[0] = editor_camera;
    world->active_camera = editor_camera;
}

bool load_scene(World* world, const char* filename) {
    // Read file
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Failed to open scene file: %s\n", filename);
        return false;
    }

    // Get file size and read it into a string
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* json_string = malloc(fsize + 1);
    fread(json_string, fsize, 1, f);
    fclose(f);
    json_string[fsize] = 0;

    // Parse JSON
    cJSON* root = cJSON_Parse(json_string);
    free(json_string);
    if (!root) {
        printf("Failed to parse scene JSON\n");
        return false;
    }

    clear_scene(world);

    cJSON* entities = cJSON_GetObjectItem(root, "entities");
    if (entities) {
        cJSON* entity;
        cJSON_ArrayForEach(entity, entities) {
            cJSON* components = cJSON_GetObjectItem(entity, "components");
            if (!components) continue;

            Entity* new_entity = create_entity(world);
            if (!new_entity) continue;

            vec3 pos;
            cJSON* transform = cJSON_GetObjectItem(components, "transform");
            if (transform) {
                cJSON *position = cJSON_GetObjectItem(transform, "position");
                if (position) {
                    pos[0] = cJSON_GetArrayItem(position, 0)->valuedouble;
                    pos[1] = cJSON_GetArrayItem(position, 1)->valuedouble;
                    pos[2] = cJSON_GetArrayItem(position, 2)->valuedouble;
                    entity_set_transform_xyz(new_entity, pos);
                }
            }

            char* texture_path;
            cJSON* renderable = cJSON_GetObjectItem(components, "renderable");
            if (renderable) {
                /* const char* mesh_type = cJSON_GetObjectItem(renderable, "mesh_type")->valuestring; */
                /* const char* material_type = cJSON_GetObjectItem(renderable, "material_type")->valuestring; */
                texture_path = cJSON_GetObjectItem(renderable, "texture_path")->valuestring;

                /* if (strcmp(mesh_type, "sprite") == 0) { */
                /* } else if (strcmp(mesh_type, "cube") == 0) { */
                /*     // Here you would call your cube creation functions */
                /*     // Example: create_cube_entity(world, ...); */
                /* } */
            }

            cJSON* character = cJSON_GetObjectItem(components, "character");
            if (character) {
                const char* character_id = cJSON_GetObjectItem(character, "character_id")->valuestring;
                const char* display_name = cJSON_GetObjectItem(character, "display_name")->valuestring;
                create_character_pos(world, texture_path, character_id, display_name, pos);
            }
        }
    }

    cJSON* cameras = cJSON_GetObjectItem(root, "cameras");
    if (cameras) {
        cJSON* camera;
        cJSON_ArrayForEach(camera, cameras) {
            // TODO: do i need ID
            /* int camera_id = cJSON_GetObjectItem(camera, "id")->valueint; */
            /* const char* camera_name = cJSON_GetObjectItem(camera, "name")->valuestring; */

            // Get camera position
            cJSON* position = cJSON_GetObjectItem(camera, "position");
            vec3 camera_pos;
            camera_pos[0] = cJSON_GetArrayItem(position, 0)->valuedouble;
            camera_pos[1] = cJSON_GetArrayItem(position, 1)->valuedouble;
            camera_pos[2] = cJSON_GetArrayItem(position, 2)->valuedouble;
            float pitch = cJSON_GetObjectItem(camera, "pitch")->valuedouble;
            float yaw = cJSON_GetObjectItem(camera, "yaw")->valuedouble;

            create_camera(world, camera_pos[0], camera_pos[1], camera_pos[2],
                          pitch, yaw);
        }
    }

    cJSON_Delete(root);
    return true;
}

bool save_scene(World* world, const char* filename)
{
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        printf("Failed to create JSON object\n");
        return false;
    }

    // Add basic scene info
    cJSON_AddStringToObject(root, "scene_name", filename);
    cJSON_AddStringToObject(root, "scene_version", "1.0");

    // Create entities array
    cJSON* entities = cJSON_CreateArray();
    if (!entities) {
        printf("Failed to create entities array\n");
        cJSON_Delete(root);
        return false;
    }

    // Save each entity
    for (uint32_t i = 0; i < world->entity_count; i++) {
        Entity* entity = &world->entities[i];
        cJSON* json_entity = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_entity, "id", entity->id);

        // Components object
        cJSON* components = cJSON_CreateObject();

        // Transform component
        cJSON* transform = cJSON_CreateObject();

        cJSON* position = cJSON_CreateArray();
        cJSON_AddItemToArray(position, cJSON_CreateNumber(entity->transform.position[0]));
        cJSON_AddItemToArray(position, cJSON_CreateNumber(entity->transform.position[1]));
        cJSON_AddItemToArray(position, cJSON_CreateNumber(entity->transform.position[2]));
        cJSON_AddItemToObject(transform, "position", position);

        // Add rotation and scale
        //cJSON* rotation = cJSON_CreateArray();
        cJSON* scale = cJSON_CreateArray();
        cJSON_AddItemToArray(scale, cJSON_CreateNumber(entity->transform.scale[0]));
        cJSON_AddItemToArray(scale, cJSON_CreateNumber(entity->transform.scale[1]));
        cJSON_AddItemToArray(scale, cJSON_CreateNumber(entity->transform.scale[2]));
        cJSON_AddItemToObject(transform, "scale", scale);

        // Fill rotation and scale arrays...

        cJSON_AddItemToObject(components, "transform", transform);

        // Only add renderable if it exists
        if (entity->renderable) {
            cJSON* renderable = cJSON_CreateObject();
            cJSON_AddStringToObject(renderable, "mesh_type", "sprite");  // For now
            cJSON_AddStringToObject(renderable, "material_type", "textured");
            if (entity->renderable->asset_path) {
                cJSON_AddStringToObject(renderable, "texture_path", entity->renderable->asset_path);
            }
            cJSON_AddItemToObject(components, "renderable", renderable);
        }

        if (entity->character) {
            cJSON* character = cJSON_CreateObject();
            cJSON_AddNumberToObject(character, "id", entity->character->id);
            cJSON_AddStringToObject(character, "character_id", strdup(entity->character->character_id));
            cJSON_AddStringToObject(character, "display_name", strdup(entity->character->display_name));
            if (entity->character->cam) {
                cJSON_AddNumberToObject(character, "camera_id", entity->character->cam->id);
            }
            cJSON_AddItemToObject(components, "character", character);
        }

        cJSON_AddItemToObject(json_entity, "components", components);
        cJSON_AddItemToArray(entities, json_entity);
    }

    cJSON_AddItemToObject(root, "entities", entities);

    cJSON* cameras = cJSON_CreateArray();
    if (!cameras) {
        printf("Failed to create cameras array\n");
        cJSON_Delete(root);
        return false;
    }

    // Add cameras (skip index 0 as it's the editor camera)
    for (int i = 1; i < world->camera_count; i++) {
        Camera* cam = &world->cameras[i];
        cJSON* camera = cJSON_CreateObject();

        cJSON_AddNumberToObject(camera, "id", cam->id);
        cJSON_AddStringToObject(camera, "name", cam->name);

        // Add position as array
        cJSON* position = cJSON_CreateArray();
        cJSON_AddItemToArray(position, cJSON_CreateNumber((float)((int)(cam->position[0] * 100)) / 100));
        cJSON_AddItemToArray(position, cJSON_CreateNumber((float)((int)(cam->position[1] * 100)) / 100));
        cJSON_AddItemToArray(position, cJSON_CreateNumber((float)((int)(cam->position[2] * 100)) / 100));
        cJSON_AddItemToObject(camera, "position", position);
        cJSON_AddItemToObject(camera, "pitch", cJSON_CreateNumber((float)((int)(cam->pitch * 100)) / 100));
        cJSON_AddItemToObject(camera, "yaw", cJSON_CreateNumber((float)((int)(cam->yaw * 100)) / 100));


        cJSON_AddItemToArray(cameras, camera);
    }

    cJSON_AddItemToObject(root, "cameras", cameras);

    // Write to file
    char* json_string = cJSON_Print(root);
    FILE* f = fopen(filename, "w");
    if (f) {
        fputs(json_string, f);
        fclose(f);
    }

    free(json_string);
    cJSON_Delete(root);

    return true;
}
