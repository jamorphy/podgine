#pragma once

#include "types.h"
#include "ecs.h"
#include "camera.h"
#include "../libs/cJSON/cJSON.h"

void parse_script(cJSON *json);
Character* get_character(World *world, const char* character_id);
void switch_to_character_camera(World *world, const char* character_id);
Entity* create_character(World* world, const char* image_path, const char* character_id, const char* display_name);
Entity* create_character_pos(World* world, const char* image_path, const char* character_id, const char* display_name, vec3 pos);
Entity* create_character_poscam(World* world,
                         const char* image_path,
                         const char* character_id,
                         const char* display_name,
                                vec3 xyz_pos, vec3 cam_pos, vec2 cam_py);
