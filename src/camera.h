#pragma once

#include "types.h"

void update_camera_frame(World* world);
Camera* create_camera(World* world,
                   float x, float y, float z,
                   float pitch, float yaw);
void create_and_add_camera(World* world,
                   float x, float y, float z,
                   float pitch, float yaw,
                   const char* name);
void create_camera_at_current_position(World* world);

void switch_camera(World *world, uint32_t camera_id);
void switch_to_character_camera(World *world, const char* character_id);

void init_camera_renderable(World* world);
