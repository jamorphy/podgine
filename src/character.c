#include <stdlib.h>
#include "character.h"
#include "ecs.h"
#include "camera.h"
#include "utils.h"

/* void parse_script(World* world, cJSON *json) { */
/*     // Get the main objects */
/*     cJSON *topic = cJSON_GetObjectItem(json, "topic"); */
/*     cJSON *timestamp = cJSON_GetObjectItem(json, "timestamp"); */
/*     cJSON *dialogue = cJSON_GetObjectItem(json, "dialogue"); */

/*     if (topic && timestamp) { */
/*         printf("Topic: %s\n", topic->valuestring); */
/*         printf("Timestamp: %s\n", timestamp->valuestring); */
/*     } */

/*     // Parse dialogue array */
/*     if (dialogue) { */
/*         int dialogue_count = cJSON_GetArraySize(dialogue); */
        
/*         for (int i = 0; i < dialogue_count; i++) { */
/*             cJSON *dialogue_item = cJSON_GetArrayItem(dialogue, i); */
            
/*             cJSON *character = cJSON_GetObjectItem(dialogue_item, "character"); */
/*             cJSON *text = cJSON_GetObjectItem(dialogue_item, "text"); */
/*             cJSON *audio_file = cJSON_GetObjectItem(dialogue_item, "audio_file"); */
            
/*             if (character && text && audio_file) { */
/*                 printf("\nDialogue Entry %d:\n", i); */
/*                 printf("Character: %s\n", character->valuestring); */
/*                 printf("Text: %s\n", text->valuestring); */
/*                 printf("Audio File: %s\n", audio_file->valuestring); */
/*             } */
/*         } */
/*     } */
/* } */

void parse_script(World* world, cJSON *json) {
    // Allocate the script if it doesn't exist
    if (!world->script) {
        world->script = (Script*)malloc(sizeof(Script));
    }

    // Get the dialogue array
    cJSON *dialogue = cJSON_GetObjectItem(json, "dialogue");
    
    if (dialogue) {
        int dialogue_count = cJSON_GetArraySize(dialogue);
        printf("Found %d dialogue entries\n", dialogue_count);
        
        // Allocate space for all lines
        world->script->lines = (Line*)malloc(sizeof(Line) * dialogue_count);
        world->script->line_count = dialogue_count;
        
        // Parse each dialogue entry
        for (int i = 0; i < dialogue_count; i++) {
            cJSON *dialogue_item = cJSON_GetArrayItem(dialogue, i);
            
            world->script->lines[i].character = strdup(cJSON_GetObjectItem(dialogue_item, "character")->valuestring);
            world->script->lines[i].text = strdup(cJSON_GetObjectItem(dialogue_item, "text")->valuestring);
            world->script->lines[i].audio_file = strdup(cJSON_GetObjectItem(dialogue_item, "audio_file")->valuestring);
            
            // Debug print during parsing
            printf("Parsing line %d:\n", i);
            if (world->script->lines[i].character) 
                printf("  Character: %s\n", world->script->lines[i].character);
            if (world->script->lines[i].text)
                printf("  Text: %s\n", world->script->lines[i].text);
            if (world->script->lines[i].audio_file)
                printf("  Audio: %s\n", world->script->lines[i].audio_file);
        }
    }
}

void cleanup_script(Script* script) {
    if (script) {
        for (int i = 0; i < script->line_count; i++) {
            free(script->lines[i].character);
            free(script->lines[i].text);
            free(script->lines[i].audio_file);
        }
        free(script->lines);
        free(script);
    }
}


Character* get_character(World *world, const char* character_id)
{
    for (uint32_t i = 0; i < world->entity_count; ++i) {
        Entity* entity = &world->entities[i];
        if (entity->character) {
            if (strcmp(entity->character->character_id, character_id) == 0) {
                return entity->character;
            }
        }
    }
    return NULL;
}

Entity* create_character(World* world,
                         const char* image_path,
                         const char* character_id,
                         const char* display_name) {
    Entity* entity = create_entity(world);
    if (!entity) return NULL;

    Character* character = (Character*)malloc(sizeof(Character));
    character->character_id = character_id;
    character->display_name = display_name;
    character->cam = create_camera(world, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f);
    entity->character = character;

    if (world->renderable_count >= 1000) {
        printf("Exceeded max renderable count\n");
        return NULL;
    }

    entity_set_transform_xyz(entity, (vec3){0.0f, 0.0f, -15.0f});
    entity_set_transform_scale(entity, (vec3){20.0f, 10.0f, 20.0f});

    Renderable* renderable = &world->renderables[world->renderable_count++];
    entity->renderable = renderable;

    sg_image texture = create_image_texture(image_path);

    renderable->mesh = create_quad_mesh(texture);
    renderable->material = create_textured_material();

    return entity;
}

Entity* create_character_pos(World* world,
                         const char* image_path,
                         const char* character_id,
                         const char* display_name,
                         vec3 pos) {
    Entity* entity = create_entity(world);
    if (!entity) return NULL;

    Character* character = (Character*)malloc(sizeof(Character));
    character->character_id = character_id;
    character->display_name = display_name;
    character->cam = create_camera(world, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f);
    entity->character = character;

    if (world->renderable_count >= 1000) {
        printf("Exceeded max renderable count\n");
        return NULL;
    }

    entity_set_transform_xyz(entity, (vec3){pos[0], pos[1], pos[2]});
    entity_set_transform_scale(entity, (vec3){25.0f, 25.0f, 25.0f});

    Renderable* renderable = &world->renderables[world->renderable_count++];
    entity->renderable = renderable;

    sg_image texture = create_image_texture(image_path);
    renderable->mesh = create_quad_mesh(texture);
    renderable->material = create_textured_material();

    return entity;
}

Entity* create_character_poscam(World* world,
                         const char* image_path,
                         const char* character_id,
                         const char* display_name,
                         vec3 xyz_pos, vec3 cam_pos, vec2 cam_py) {
    Entity* entity = create_entity(world);
    if (!entity) return NULL;

    Character* character = (Character*)malloc(sizeof(Character));
    character->character_id = character_id;
    character->display_name = display_name;
    character->cam = create_camera(world, cam_pos[0], cam_pos[1], cam_pos[2], cam_py[0], cam_py[1]);
    entity->character = character;

    if (world->renderable_count >= 1000) {
        printf("Exceeded max renderable count\n");
        return NULL;
    }

    entity_set_transform_xyz(entity, (vec3){xyz_pos[0], xyz_pos[1], xyz_pos[2]});
    entity_set_transform_scale(entity, (vec3){25.0f, 25.0f, 25.0f});

    Renderable* renderable = &world->renderables[world->renderable_count++];
    entity->renderable = renderable;

    sg_image texture = create_image_texture(image_path);
    renderable->mesh = create_quad_mesh(texture);
    renderable->material = create_textured_material();

    return entity;
}
