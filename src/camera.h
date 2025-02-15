#pragma once

#include "types.h"
#include "ecs.h"

/* void create_camera(World* world, */
/*                    float distance, */
/*                    float pitch, */
/*                    float yaw, */
/*                    float x, */
/*                    float y, */
/*                    float z, */
/*                    const char* name); */
void update_camera_frame(World* world);
Camera* create_camera(World* world,
                   float x, float y, float z,
                   float pitch, float yaw);
void create_and_add_camera(World* world,
                   float x, float y, float z,
                   float pitch, float yaw,
                   const char* name);
void create_camera_at_current_position(World* world);
