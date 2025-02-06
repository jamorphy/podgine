#include "ecs.h"

#include <stdlib.h>
#include <stdio.h>

Entity* create_entity(World *world) {
    if (world->entity_count >= 1000) return NULL;

    Entity* entity = &world->entities[world->entity_count];
    entity->id = world->next_entity_id++;
    entity->transform = NULL;
    entity->mesh = NULL;

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
    transform->scale[0] = 1.0f;
    transform->scale[1] = 1.0f;
    transform->scale[2] = 1.0f;
    
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

    mesh->vertex_count = 36;  // 6 faces * 2 triangles * 3 vertices
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
    mesh->index_buffer.id = 0; // No index buffer for grid

    free(vertices);
    entity->mesh = mesh;
    return mesh;
}

Entity* create_grid_entity(World* world) {
    Entity* entity = create_entity(world);
    add_transform(world, entity, 0.0f, 0.0f, 0.0f);    
    add_grid_mesh(world, entity);
    return entity;
}
