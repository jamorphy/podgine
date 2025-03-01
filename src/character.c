#include <stdlib.h>
#include "log.h"
#include "character.h"
#include "ecs.h"
#include "camera.h"
#include "utils.h"

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
        LOG_FATAL("Exceeded MAX_RENDERABLES");
        return NULL;
    }

    entity_set_transform_xyz(entity, (vec3){0.0f, 0.0f, -15.0f});
    entity_set_transform_scale(entity, (vec3){20.0f, 10.0f, 20.0f});

    Renderable* renderable = &world->renderables[world->renderable_count++];
    renderable->asset_path = strdup(image_path);
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
    character->character_id = strdup(character_id);
    character->display_name = strdup(display_name);
    character->cam = create_camera(world, 5.0, 5.0, 5.0, 5.0, 5.0);
    entity->character = character;

    if (world->renderable_count >= 1000) {
        LOG_FATAL("Exceeded MAX_RENDERABLES");
        return NULL;
    }

    entity_set_transform_xyz(entity, (vec3){pos[0], pos[1], pos[2]});
    entity_set_transform_scale(entity, (vec3){25.0f, 25.0f, 25.0f});

    Renderable* renderable = &world->renderables[world->renderable_count++];
    renderable->asset_path = strdup(image_path);
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
        LOG_FATAL("Exceeded MAX_RENDERABLES");
        return NULL;
    }

    entity_set_transform_xyz(entity, (vec3){xyz_pos[0], xyz_pos[1], xyz_pos[2]});
    entity_set_transform_scale(entity, (vec3){25.0f, 25.0f, 25.0f});

    Renderable* renderable = &world->renderables[world->renderable_count++];
    renderable->asset_path = strdup(image_path);
    entity->renderable = renderable;

    sg_image texture = create_image_texture(image_path);
    renderable->mesh = create_quad_mesh(texture);
    renderable->material = create_textured_material();

    return entity;
}
