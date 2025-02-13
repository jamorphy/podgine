#include "character.h"
#include "utils.h"

Entity* create_character(World* world,
                         const char* image_path,
                         const char* character_id,
                         const char* display_name) {
    Entity* entity = create_entity(world);
    if (!entity) return NULL;

    Character* character = (Character*)malloc(sizeof(Character));
    character->character_id = character_id;
    character->display_name = display_name;
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
