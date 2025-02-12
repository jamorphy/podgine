#include "character.h"

Entity* create_character(World* world, const char* character_id, const char* display_name) {
    Entity* entity = create_entity(world);
    if (!entity) return NULL;

    entity->character->character_id = character_id;
    entity->character->display_name = display_name;

    return entity;
}
