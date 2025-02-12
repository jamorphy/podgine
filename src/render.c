#include "render.h"

void compute_model_matrix(Transform* transform, mat4x4 out_matrix)
{
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

void render_grid(World* world, mat4x4 view, mat4x4 proj)
{
    // Start with identity model matrix (grid is at world origin)
    mat4x4 model_matrix;
    mat4x4_identity(model_matrix);
    
    // Compute MVP matrix
    mat4x4 mvp_matrix;
    mat4x4_mul(mvp_matrix, proj, view);
    mat4x4_mul(mvp_matrix, mvp_matrix, model_matrix);

    // Set up rendering state
    sg_apply_pipeline(world->grid_renderable.material->pipeline);
    
    // Create bindings for the grid mesh
    sg_bindings bindings = {
        .vertex_buffers[0] = world->grid_renderable.mesh->vertex_buffer
    };
    sg_apply_bindings(&bindings);

    // Apply MVP matrix uniform
    vs_params_t vs_params;
    memcpy(vs_params.mvp, mvp_matrix, sizeof(mvp_matrix));
    sg_apply_uniforms(0, &SG_RANGE(vs_params));

    // Draw the grid lines
    sg_draw(0, world->grid_renderable.mesh->vertex_count, 1);
}

void render_cameras(World* world, mat4x4 view, mat4x4 proj)
{
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

void render_entities(World* world, mat4x4 view, mat4x4 proj)
{
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

