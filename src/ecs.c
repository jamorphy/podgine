#include "ecs.h"

#include <stdlib.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb_image.h"

#define COMPONENTS_PER_VERTEX 7 // 3 pos floats + 4 color floats

/* Mesh* add_grid_mesh(World* world, Entity* entity) { */
/*     const int GRID_SIZE = 20; */
/*     const float GRID_SPACE = 2.0f; */
/*     const int LINES_PER_DIR = (GRID_SIZE * 2 + 1); */
/*     const int VERTS_PER_LINE = 2; */
/*     const int TOTAL_LINES = LINES_PER_DIR * 2; */
/*     const int VERTEX_COUNT = TOTAL_LINES * VERTS_PER_LINE; */

/*     float* vertices = (float*)malloc(VERTEX_COUNT * 6 * sizeof(float)); */
/*     int vertex_idx = 0; */

/*     // Create grid lines */
/*     for (int i = -GRID_SIZE; i <= GRID_SIZE; i++) { */
/*         float color_intensity = 1.0f; */

/*         // Lines along X axis */
/*         vertices[vertex_idx++] = i * GRID_SPACE; */
/*         vertices[vertex_idx++] = -0.01f; */
/*         vertices[vertex_idx++] = -GRID_SIZE * GRID_SPACE; */
/*         vertices[vertex_idx++] = color_intensity; */
/*         vertices[vertex_idx++] = color_intensity; */
/*         vertices[vertex_idx++] = color_intensity; */

/*         vertices[vertex_idx++] = i * GRID_SPACE; */
/*         vertices[vertex_idx++] = 0.0f; */
/*         vertices[vertex_idx++] = GRID_SIZE * GRID_SPACE; */
/*         vertices[vertex_idx++] = color_intensity; */
/*         vertices[vertex_idx++] = color_intensity; */
/*         vertices[vertex_idx++] = color_intensity; */

/*         // Lines along Z axis */
/*         vertices[vertex_idx++] = -GRID_SIZE * GRID_SPACE; */
/*         vertices[vertex_idx++] = 0.0f; */
/*         vertices[vertex_idx++] = i * GRID_SPACE; */
/*         vertices[vertex_idx++] = color_intensity; */
/*         vertices[vertex_idx++] = color_intensity; */
/*         vertices[vertex_idx++] = color_intensity; */

/*         vertices[vertex_idx++] = GRID_SIZE * GRID_SPACE; */
/*         vertices[vertex_idx++] = 0.0f; */
/*         vertices[vertex_idx++] = i * GRID_SPACE; */
/*         vertices[vertex_idx++] = color_intensity; */
/*         vertices[vertex_idx++] = color_intensity; */
/*         vertices[vertex_idx++] = color_intensity; */
/*     } */

/*     Mesh* mesh = &world->meshes[entity->id]; */
/*     mesh->vertex_buffer = sg_make_buffer(&(sg_buffer_desc){ */
/*             .data = (sg_range) { */
/*                 .ptr = vertices, */
/*                 .size = vertex_idx * sizeof(float) */
/*             }, */
/*         .label = "grid-vertices" */
/*     }); */
/*     mesh->vertex_count = vertex_idx / 6; */
/*     mesh->index_count = 0; */
/*     mesh->index_buffer.id = 0; */

/*     free(vertices); */
/*     entity->mesh = mesh; */
/*     return mesh; */
/* } */

Mesh* create_quad_mesh(sg_image texture) {
    Mesh* mesh = malloc(sizeof(Mesh));
    if (!mesh) {
        printf("Failed to allocate quad mesh\n");
        return NULL;
    }

    float vertices[] = {
        // positions         // UVs
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f,
        0.5f, -0.5f, 0.0f,   1.0f, 1.0f,
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f,
    };

    uint16_t indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    mesh->vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .label = "quad-vertices"
    });

    mesh->index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "quad-indices"
    });

    mesh->bindings.vertex_buffers[0] = mesh->vertex_buffer;
    mesh->bindings.index_buffer = mesh->index_buffer;
    mesh->index_count = 6;

    mesh->bindings.images[0] = texture;
    mesh->bindings.samplers[0] = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
    });

    return mesh;
}



Mesh* create_cube_mesh(void) {

    Mesh* mesh = malloc(sizeof(Mesh));
    if (!mesh) {
        printf("Failed to allocated a mesh\n");
        return NULL;
    }

    float vertices[] = {
        // Positions          // Colors (RGBA)
        // Front face
        -1.0f, -1.0f,  1.0f,   1.0f, 0.0f, 0.0f, 1.0f,  // Bottom-left-front (red)
         1.0f, -1.0f,  1.0f,   0.0f, 1.0f, 0.0f, 1.0f,  // Bottom-right-front (green)
         1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f, 1.0f,  // Top-right-front (blue)
        -1.0f,  1.0f,  1.0f,   1.0f, 1.0f, 0.0f, 1.0f,  // Top-left-front (yellow)

        // Back face
        -1.0f, -1.0f, -1.0f,   1.0f, 0.0f, 1.0f, 1.0f,  // Bottom-left-back (magenta)
         1.0f, -1.0f, -1.0f,   0.0f, 1.0f, 1.0f, 1.0f,  // Bottom-right-back (cyan)
         1.0f,  1.0f, -1.0f,   0.5f, 0.5f, 0.5f, 1.0f,  // Top-right-back (gray)
        -1.0f,  1.0f, -1.0f,   0.0f, 0.0f, 0.0f, 1.0f   // Top-left-back (black)
    };
    uint16_t indices[] = {
        // Front face
        0, 1, 2,   0, 2, 3,
        // Right face
        1, 5, 6,   1, 6, 2,
        // Back face
        5, 4, 7,   5, 7, 6,
        // Left face
        4, 0, 3,   4, 3, 7,
        // Bottom face
        4, 5, 1,   4, 1, 0,
        // Top face
        3, 2, 6,   3, 6, 7
    };

    mesh->vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .label = "cube-vertices"
    });

    mesh->index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "cube-indices"
    });

    mesh->bindings.vertex_buffers[0] = mesh->vertex_buffer;
    mesh->bindings.index_buffer = mesh->index_buffer;

    mesh->index_count = sizeof(indices) / sizeof(uint16_t);

    return mesh;
}

Material* create_textured_material(void) {
    Material* material = malloc(sizeof(Material));
    if (!material) {
        printf("Failed to allocate material\n");
        return NULL;
    }

    char* vs_source = read_shader_file("src/shaders/texture_vs.metal");
    char* fs_source = read_shader_file("src/shaders/texture_fs.metal");

    material->shader = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0,
        },
        .images[0] = {
            .stage = SG_SHADERSTAGE_FRAGMENT,
            .msl_texture_n = 0
        },
        .samplers[0] = {
            .stage = SG_SHADERSTAGE_FRAGMENT,
            .msl_sampler_n = 0
        },
        .image_sampler_pairs[0] = {
            .stage = SG_SHADERSTAGE_FRAGMENT,
            .image_slot = 0,
            .sampler_slot = 0
        },
        .vertex_func = {
            .entry = "vs_main",
            .source = vs_source
        },
        .fragment_func = {
            .entry = "fs_main",
            .source = fs_source
        },
        //.label = "texture2d-shader"
    });

    material->pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = material->shader,
        .layout = {
            .buffers[0].stride = sizeof(float)*5, // 3 floats for pos, 2 for uv
            .attrs = {
                [0] = { .format = SG_VERTEXFORMAT_FLOAT3, .offset = 0 },
                [1] = { .format = SG_VERTEXFORMAT_FLOAT2, .offset = sizeof(float)*3 },
            }
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .label = "textured-quad-pipeline"
    });

    return material;
}

Material* create_cube_material(void) {

    Material* material = malloc(sizeof(Material));
    if (!material) {
        printf("Failed to allocated material\n");
        return NULL;
    }

    // Load the shaders (vertex and fragment shaders for the cube)
    char* vs_source = read_shader_file("src/shaders/cube_vs.metal");
    char* fs_source = read_shader_file("src/shaders/cube_fs.metal");

    material->shader = sg_make_shader(&(sg_shader_desc){
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(vs_params_t),
            .msl_buffer_n = 0, // Match this to your shader code
        },
        .vertex_func = { .source = vs_source },
        .fragment_func = { .source = fs_source },
        .label = "cube-shader"
    });

    material->pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = material->shader,
        .layout = {
            .attrs = {
                [0] = { .format = SG_VERTEXFORMAT_FLOAT3 }, // Position
                [1] = { .format = SG_VERTEXFORMAT_FLOAT4 }, // Color
            }
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_FRONT,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
        },
        .label = "cube-pipeline"
    });

    // Free the shader sources if allocated
    free(vs_source);
    free(fs_source);

    return material;
}

void init_camera_visualization(World* world)  {
    // Create a material for camera visualization using your existing cube material
    Material* camera_material = create_cube_material();
    
    // Store the material in your world's materials array
    world->materials[world->material_count] = *camera_material;
    free(camera_material);  // Free the allocated material after copying
    
    // Initialize the dedicated camera visualization renderable
    world->camera_visualization_renderable.material = &world->materials[world->material_count];
    world->camera_visualization_renderable.mesh = create_cube_mesh();
    
    world->material_count++;
    // Note: we no longer increment renderable_count since we're not using the global array
}



void destroy_mesh(Mesh* mesh) {
    if (mesh) {
        sg_destroy_buffer(mesh->vertex_buffer);
        sg_destroy_buffer(mesh->index_buffer);
        free(mesh);
    }
}

void destroy_material(Material* material) {
    if (material) {
        sg_destroy_shader(material->shader);
        sg_destroy_pipeline(material->pipeline);
        free(material);
    }
}



Entity* create_entity(World *world) {
    if (world->entity_count >= 1000) {
        printf("Exceeded max entity count\n");
        return NULL;
    }

    Entity* entity = &world->entities[world->entity_count];
    entity->id = world->next_entity_id++;

    entity->transform = (Transform){0};
    entity->transform.scale[0] = 1.0f;
    entity->transform.scale[1] = 1.0f;
    entity->transform.scale[2] = 1.0f;

    entity->renderable = NULL;

    world->entity_count++;
    return entity;
};

void destroy_entity(World *world, Entity *entity) {
    size_t index = entity - world->entities;

    // Validate index
    if (index >= world->entity_count) {
        printf("ERROR: Invalid entity pointer (index %zu >= count %d)\n",
               index, world->entity_count);
        return;
    }

    // Clean up renderable reference
    if (entity->renderable) {
        // Instead of destroying mesh/material directly,
        // we might want to reference count or handle shared resources differently
        entity->renderable = NULL;
    }

    // Move the last entity into this slot (if it's not already the last one)
    if (index < world->entity_count - 1) {
        world->entities[index] = world->entities[world->entity_count - 1];
    }

    // Decrease the count
    world->entity_count--;
}

Entity* create_img(World* world, const char* image_path, vec3 pos, vec3 scale) {
    Entity* entity = create_entity(world);
    if (!entity) return NULL;

    // Set transform
    entity->transform.position[0] = pos[0];
    entity->transform.position[1] = pos[1];
    entity->transform.position[2] = pos[2];

    entity->transform.scale[0] = scale[0];
    entity->transform.scale[1] = scale[1];
    entity->transform.scale[2] = scale[2];

    entity->transform.rotation[0] = 0.0f;
    entity->transform.rotation[1] = 0.0f;
    entity->transform.rotation[2] = 0.0f;

    int img_width, img_height, img_channels;
    unsigned char* img_data = stbi_load(image_path, &img_width, &img_height, &img_channels, 4);
    if (!img_data) {
        printf("Failed to load image: %s\n", image_path);
        return NULL;
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

    // Create renderable
    if (world->renderable_count >= 1000) {
        printf("Exceeded max renderable count\n");
        return NULL;
    }

    Renderable* renderable = &world->renderables[world->renderable_count++];
    entity->renderable = renderable;

    // Set up mesh and material
    renderable->mesh = create_quad_mesh(texture);
    renderable->material = create_textured_material();

    return entity;
}


Entity* create_cube(World* world, vec3 pos, vec3 scale) {

    Entity* entity = create_entity(world);
    if (!entity) return NULL;

    entity->transform.position[0] = pos[0];
    entity->transform.position[1] = pos[1];
    entity->transform.position[2] = pos[2];

    entity->transform.scale[0] = scale[0];
    entity->transform.scale[1] = scale[1];
    entity->transform.scale[2] = scale[2];

    if (world->renderable_count >= 1000) {
        printf("Exceeded max renderable count\n");
        return NULL;
    }
    Renderable* renderable = &world->renderables[world->renderable_count++];
    entity->renderable = renderable;

    // Set up the mesh and material
    renderable->mesh = create_cube_mesh();
    renderable->material = create_cube_material();

    return entity;
}

// Function to compute the model matrix from the Transform component
void compute_model_matrix(Transform* transform, mat4x4 out_matrix) {
    mat4x4_identity(out_matrix);

    // Apply translation
    mat4x4_translate_in_place(out_matrix, transform->position[0], transform->position[1], transform->position[2]);

    // Apply rotation (assuming Euler angles in radians)
    mat4x4 rotation;
    mat4x4_identity(rotation);
    mat4x4_rotate_X(rotation, rotation, transform->rotation[0]);
    mat4x4_rotate_Y(rotation, rotation, transform->rotation[1]);
    mat4x4_rotate_Z(rotation, rotation, transform->rotation[2]);
    mat4x4_mul(out_matrix, out_matrix, rotation);

    // Apply scaling
    mat4x4_scale_aniso(out_matrix, out_matrix, transform->scale[0], transform->scale[1], transform->scale[2]);
}

Mesh create_camera_mesh(void) {
    // Simple pyramid-like shape for camera representation
    float vertices[] = {
        // Position (x, y, z)        // Color (r, g, b)
        0.0f,  0.0f,  0.0f,         1.0f, 0.0f, 0.0f,  // base center
        -0.5f, -0.5f, -1.0f,        1.0f, 0.0f, 0.0f,  // base corners
        0.5f, -0.5f, -1.0f,         1.0f, 0.0f, 0.0f,
        0.5f,  0.5f, -1.0f,         1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -1.0f,        1.0f, 0.0f, 0.0f,
    };

    uint16_t indices[] = {
        0, 1, 2,  // sides
        0, 2, 3,
        0, 3, 4,
        0, 4, 1,
        1, 2, 3,  // base
        1, 3, 4
    };

    sg_buffer_desc vbuf_desc = {
        .data = SG_RANGE(vertices),
        .label = "camera-vertices"
    };

    sg_buffer_desc ibuf_desc = {
        .data = SG_RANGE(indices),
        .label = "camera-indices"
    };

    Mesh mesh = {
        .vertex_buffer = sg_make_buffer(&vbuf_desc),
        .index_buffer = sg_make_buffer(&ibuf_desc),
        .index_count = sizeof(indices) / sizeof(indices[0])
    };

    mesh.bindings.vertex_buffers[0] = mesh.vertex_buffer;
    mesh.bindings.index_buffer = mesh.index_buffer;

    return mesh;
}

void render_cameras(World* world, mat4x4 view, mat4x4 proj) {
    for (int i = 0; i < world->camera_count; i++) {
        Camera* camera = &world->cameras[i];

        // Start with identity
        mat4x4 model_matrix;
        mat4x4_identity(model_matrix);

        mat4x4_translate_in_place(model_matrix, 
            camera->position[0],
            camera->position[1],
            camera->position[2]);
        
        // Then rotate
        mat4x4 temp;
        float pitch_rad = camera->pitch * (M_PI / 180.0f);
        float yaw_rad = camera->yaw * (M_PI / 180.0f);
        
        mat4x4_rotate_X(temp, model_matrix, pitch_rad);
        mat4x4_rotate_Y(model_matrix, temp, yaw_rad);

        // First scale (small cube)
        mat4x4_scale(model_matrix, model_matrix, 0.25f);
        
        // MVP computation
        mat4x4 mvp_matrix;
        mat4x4_mul(mvp_matrix, proj, view);
        mat4x4_mul(mvp_matrix, mvp_matrix, model_matrix);

        Renderable* camera_renderable = &world->camera_visualization_renderable;
        
        sg_apply_pipeline(camera_renderable->material->pipeline);
        sg_apply_bindings(&camera_renderable->mesh->bindings);

        vs_params_t vs_params;
        memcpy(vs_params.mvp, mvp_matrix, sizeof(mvp_matrix));
        sg_apply_uniforms(0, &SG_RANGE(vs_params));

        sg_draw(0, camera_renderable->mesh->index_count, 1);
    }
}

void render_entities(World* world, mat4x4 view, mat4x4 proj) {
    for (uint32_t i = 0; i < world->entity_count; ++i) {
        Entity* entity = &world->entities[i];

        if (entity->renderable) {
            // Compute the model matrix from the Transform component
            mat4x4 model_matrix;
            compute_model_matrix(&entity->transform, model_matrix);

            // Compute the MVP matrix
            mat4x4 mvp_matrix;
            mat4x4_mul(mvp_matrix, proj, view);
            mat4x4_mul(mvp_matrix, mvp_matrix, model_matrix);

            // Set the pipeline state object (PSO) for the material
            sg_apply_pipeline(entity->renderable->material->pipeline);

            // Bind the mesh's vertex and index buffers
            sg_apply_bindings(&entity->renderable->mesh->bindings);

            // Prepare uniform data
            vs_params_t vs_params;
            memcpy(vs_params.mvp, mvp_matrix, sizeof(mvp_matrix));

            // Apply uniforms with correct stage and uniform block index
            sg_apply_uniforms(0, &SG_RANGE(vs_params));

            // Issue the draw call
            sg_draw(0, entity->renderable->mesh->index_count, 1);
        }
    }
}

