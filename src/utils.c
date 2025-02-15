#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb_image.h"

char* read_text_file(const char* filepath)
{
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Failed to open shader file: %s\n", filepath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        printf("Failed to allocate memory for shader\n");
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    
    fclose(file);
    return buffer;
}

sg_image create_image_texture(const char* filepath)
{
    int img_width, img_height, img_channels;
    unsigned char* img_data = stbi_load(filepath, &img_width, &img_height, &img_channels, 4);
    if (!img_data) {
        printf("Failed to load image: %s\n", filepath);
    }

    // Create texture
    sg_image texture = sg_make_image(&(sg_image_desc){
        .width = img_width,
        .height = img_height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = {
            .ptr = img_data,
            .size = (size_t)(img_width * img_height * 4)
        }
    });

    stbi_image_free(img_data);
    return texture;
}
