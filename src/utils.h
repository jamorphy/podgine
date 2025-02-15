#pragma once

#include "../libs/sokol/sokol_gfx.h"

char* read_text_file(const char* filepath);
sg_image create_image_texture(const char* filepath);
