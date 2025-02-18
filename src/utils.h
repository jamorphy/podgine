#pragma once

#include "../libs/sokol/sokol_gfx.h"

typedef enum {
    ERROR_NONE = 0,
    ERROR_MAX_CAMERAS_REACHED,
    ERROR_INVALID_WORLD,
    ERROR_OUT_OF_MEMORY
} Error;

typedef enum {
    SEVERITY_NONE = 0,
    SEVERITY_WARNING,
    SEVERITY_ERROR
} ErrorSev;

char* read_text_file(const char* filepath);
sg_image create_image_texture(const char* filepath);
