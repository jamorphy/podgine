#include "ecs.h"

#include <stdlib.h>
#include <stdio.h>

#define COMPONENTS_PER_VERTEX 7 // 3 pos floats + 4 color floats

char* read_shader_file(const char* filepath) {
    printf("Attempting to load shader: %s\n", filepath);
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



Entity* create_entity(World *world) {
    if (world->entity_count >= 1000) return NULL;

    Entity* entity = &world->entities[world->entity_count];
    entity->id = world->next_entity_id++;
    entity->transform = NULL;
    entity->mesh = NULL;
    entity->renderable = NULL;

    world->entity_count++;
    return entity;
};

Transform* add_transform(World *world, Entity *entity, float x, float y, float z) {
    Transform* transform = &world->transforms[entity->id];
    
    transform->position[0] = x;
    transform->position[1] = y;
    transform->position[2] = z;
    
    // Initialize rotation and scale
    transform->rotation[0] = 0.0f;
    transform->rotation[1] = 0.0f;
    transform->rotation[2] = 0.0f;
    transform->scale[0] = 2.0f;
    transform->scale[1] = 2.0f;
    transform->scale[2] = 2.0f;
    
    entity->transform = transform;
    return transform;
}

Mesh* add_cube_mesh(World* world, Entity* entity) {
    Mesh* mesh = &world->meshes[entity->id];

    float vertices[] = {
        -1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
         1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
         1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
        -1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,

        -1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
         1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
         1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
        -1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,

        -1.0, -1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0,  1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0,  1.0,  1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0, -1.0,  1.0,   0.0, 0.0, 1.0, 1.0,

        1.0, -1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
        1.0,  1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
        1.0,  1.0,  1.0,    1.0, 0.5, 0.0, 1.0,
        1.0, -1.0,  1.0,    1.0, 0.5, 0.0, 1.0,

        -1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,
        -1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
         1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
         1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,

        -1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0,
        -1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
         1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
         1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0
    };

    uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
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

    mesh->vertex_count = 24;  // 6 faces * 2 triangles * 3 vertices
    mesh->index_count = 36;
    entity->mesh = mesh;
    return mesh;
}

Entity* create_cube_entity(World *world, float x, float y, float z) {
    Entity *entity = create_entity(world);
    if (!entity) return NULL;

    add_transform(world, entity, x, y, z);
    add_cube_mesh(world, entity);
    return entity;
}

Mesh* add_grid_mesh(World* world, Entity* entity) {
    const int GRID_SIZE = 20;
    const float GRID_SPACE = 2.0f;
    const int LINES_PER_DIR = (GRID_SIZE * 2 + 1);
    const int VERTS_PER_LINE = 2;
    const int TOTAL_LINES = LINES_PER_DIR * 2;
    const int VERTEX_COUNT = TOTAL_LINES * VERTS_PER_LINE;
    
    float* vertices = (float*)malloc(VERTEX_COUNT * 6 * sizeof(float));
    int vertex_idx = 0;

    // Create grid lines
    for (int i = -GRID_SIZE; i <= GRID_SIZE; i++) {
        float color_intensity = 1.0f;
        
        // Lines along X axis
        vertices[vertex_idx++] = i * GRID_SPACE;
        vertices[vertex_idx++] = -0.01f;
        vertices[vertex_idx++] = -GRID_SIZE * GRID_SPACE;
        vertices[vertex_idx++] = color_intensity;
        vertices[vertex_idx++] = color_intensity;
        vertices[vertex_idx++] = color_intensity;
        
        vertices[vertex_idx++] = i * GRID_SPACE;
        vertices[vertex_idx++] = 0.0f;
        vertices[vertex_idx++] = GRID_SIZE * GRID_SPACE;
        vertices[vertex_idx++] = color_intensity;
        vertices[vertex_idx++] = color_intensity;
        vertices[vertex_idx++] = color_intensity;

        // Lines along Z axis
        vertices[vertex_idx++] = -GRID_SIZE * GRID_SPACE;
        vertices[vertex_idx++] = 0.0f;
        vertices[vertex_idx++] = i * GRID_SPACE;
        vertices[vertex_idx++] = color_intensity;
        vertices[vertex_idx++] = color_intensity;
        vertices[vertex_idx++] = color_intensity;
        
        vertices[vertex_idx++] = GRID_SIZE * GRID_SPACE;
        vertices[vertex_idx++] = 0.0f;
        vertices[vertex_idx++] = i * GRID_SPACE;
        vertices[vertex_idx++] = color_intensity;
        vertices[vertex_idx++] = color_intensity;
        vertices[vertex_idx++] = color_intensity;
    }

    printf("Grid vertex count: %d\n", vertex_idx / 6);
printf("First line vertices:\n");
for (int i = 0; i < 12; i += 6) {  // Print first two vertices (one line)
    printf("v%d: pos(%.2f, %.2f, %.2f) color(%.2f, %.2f, %.2f)\n",
        i/6,
        vertices[i], vertices[i+1], vertices[i+2],
        vertices[i+3], vertices[i+4], vertices[i+5]);
}

    Mesh* mesh = &world->meshes[entity->id];
    mesh->vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
            .data = (sg_range) {
                .ptr = vertices,
                .size = vertex_idx * sizeof(float)
            },
        .label = "grid-vertices"
    });
    mesh->vertex_count = vertex_idx / 6;
    mesh->index_count = 0;
    mesh->index_buffer.id = 0;

    free(vertices);
    entity->mesh = mesh;
    return mesh;
}

Entity* create_cube(World* world, float x, float y, float z) {
    Entity* entity = create_entity(world);
    add_transform(world, entity, x, y, z);
    add_cube_mesh(world, entity);  // Your existing function
    
    // Add render handle
    RenderHandle* render = &world->renders[entity->id];
    render->type = PIPELINE_STANDARD;
    entity->render = render;
    
    return entity;
}

Entity* create_grid(World* world) {
    Entity* entity = create_entity(world);
    add_transform(world, entity, 0, 0, 0);
    add_grid_mesh(world, entity);  // Your existing function
    
    RenderHandle* render = &world->renders[entity->id];
    render->type = PIPELINE_GRID;
    entity->render = render;
    
    return entity;
}


void create_camera(World* world,
                   float distance,
                   float pitch,
                   float yaw,
                   float x,
                   float y,
                   float z,
                   const char* name) {
    // Check if we have room for another camera
    if (world->camera_count >= 16) {
        printf("Warning: Maximum number of cameras reached\n");
        return;
    }

    Camera* new_cam = &world->cameras[world->camera_count];
    
    // Initialize camera properties
    new_cam->distance = distance;
    new_cam->pitch = pitch;
    new_cam->yaw = yaw;
    new_cam->position[0] = x;
    new_cam->position[1] = y;
    new_cam->position[2] = z;
    new_cam->id = world->next_camera_id++;
    strncpy(new_cam->name, name, 31);
    new_cam->name[31] = '\0';  // Ensure null termination
    
    // Initialize default values
    new_cam->center[0] = 0.0f;  // Looking at origin
    new_cam->center[1] = 0.0f;
    new_cam->center[2] = 0.0f;
    new_cam->up[0] = 0.0f;     // Up vector
    new_cam->up[1] = 1.0f;
    new_cam->up[2] = 0.0f;

    world->camera_count++;
}

Mesh* create_cube_mesh(World* world) {
    if (world->mesh_count >= 1000) {
        printf("Exceeded max mesh count\n");
        return NULL;
    }

    Mesh* mesh = &world->meshes[world->mesh_count++];

    // Define your cube's vertex and index data here
    // For example:
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

Material* create_cube_material(World* world) {
    if (world->material_count >= 100) {
        printf("Exceeded max material count\n");
        return NULL;
    }

    Material* material = &world->materials[world->material_count++];

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

bool add_entity_to_world(World* world, Entity* entity) {
    if (world->entity_count >= 1000) {
        fprintf(stderr, "World entity limit reached\n");
        return false;
    }

    // Assign a unique ID to the entity
    entity->id = world->next_entity_id++;

    // Copy the entity into the world's entity array
    world->entities[world->entity_count] = *entity;
    Entity* world_entity = &world->entities[world->entity_count];

    // Handle the Transform component
    if (entity->transform) {
        // Copy the transform into the world's transform pool
        world->transforms[world->entity_count] = *(entity->transform);
        // Update the entity's transform pointer to point to the world's pool
        world_entity->transform = &world->transforms[world->entity_count];
    } else {
        world_entity->transform = NULL;
    }

    // Handle the Renderable component
    if (entity->renderable) {
        // Copy the renderable into the world's renderables pool
        world->renderables[world->entity_count] = *(entity->renderable);
        // Update the entity's renderable pointer to point to the world's pool
        world_entity->renderable = &world->renderables[world->entity_count];
    } else {
        world_entity->renderable = NULL;
    }

    // Increment the entity count
    world->entity_count++;

    return true;
}

Entity* create_cube_new(World* world, float x, float y, float z) {
    if (world->entity_count >= 1000) {
        printf("Exceeded max entity count\n");
        return NULL;
    }

    // Get an Entity from the world array
    Entity* entity = &world->entities[world->entity_count++];

    // Assign an id to the entity
    entity->id = world->next_entity_id++;

    // Allocate a Transform from the world's transform pool
    if (world->transform_count >= 1000) {
        printf("Exceeded max transform count\n");
        return NULL;
    }
    Transform* transform = &world->transforms[world->transform_count++];
    transform->position[0] = x;
    transform->position[1] = y;
    transform->position[2] = z;
    transform->scale[0] = transform->scale[1] = transform->scale[2] = 5.0f;
    entity->transform = transform;

    // Allocate a Renderable from the world's renderable pool
    if (world->renderable_count >= 1000) {
        printf("Exceeded max renderable count\n");
        return NULL;
    }
    Renderable* renderable = &world->renderables[world->renderable_count++];
    entity->renderable = renderable;

    // Set up the mesh and material
    renderable->mesh = create_cube_mesh(world); // We'll modify this function next
    renderable->material = create_cube_material(world); // We'll modify this function next

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

// Function to render all entities in the world
void render_entities(World* world, mat4x4 view, mat4x4 proj) {
    for (uint32_t i = 0; i < world->entity_count; ++i) {
        Entity* entity = &world->entities[i];

        if (entity->transform && entity->renderable) {
            // Compute the model matrix from the Transform component
            mat4x4 model_matrix;
            compute_model_matrix(entity->transform, model_matrix);

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
