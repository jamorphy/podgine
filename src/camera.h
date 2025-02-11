#pragma once

#include "types.h"

/* void create_camera(World* world, */
/*                    float distance, */
/*                    float pitch, */
/*                    float yaw, */
/*                    float x, */
/*                    float y, */
/*                    float z, */
/*                    const char* name); */
void update_camera_frame(World* world);
void create_camera(World* world,
                   float x, float y, float z,
                   float pitch, float yaw,
                   const char* name);
