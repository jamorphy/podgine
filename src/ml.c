/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <math.h> */
/* #include <errno.h> */

/* #include "ml.h" */

/* // Fast approximation of tanh (for CPU efficiency) */
/* float tanh_approx(float x) { */
/*     if (x > 3.0f) return 1.0f; */
/*     if (x < -3.0f) return -1.0f; */
/*     float x2 = x * x; */
/*     return x * (27.0f + x2) / (27.0f + 9.0f * x2);  // Pade approximation */
/* } */

/* ModelParams* load_model_params(const char* filename) { */
/*     FILE* file = fopen(filename, "rb"); */
/*     if (!file) { */
/*         printf("Error: Could not open file %s\n", filename); */
/*         return NULL; */
/*     } */

/*     ModelParams* params = (ModelParams*)malloc(sizeof(ModelParams)); */
/*     if (!params) { */
/*         printf("Error: Memory allocation failed\n"); */
/*         fclose(file); */
/*         return NULL; */
/*     } */

/*     size_t expected = INPUT_DIM * HIDDEN_DIM + HIDDEN_DIM + */
/*                       HIDDEN_DIM * HIDDEN_DIM + HIDDEN_DIM + */
/*                       HIDDEN_DIM * OUTPUT_DIM + OUTPUT_DIM; */
/*     size_t read_count = 0; */
/*     read_count += fread(params->w1, sizeof(float), INPUT_DIM * HIDDEN_DIM, file); */
/*     read_count += fread(params->b1, sizeof(float), HIDDEN_DIM, file); */
/*     read_count += fread(params->w2, sizeof(float), HIDDEN_DIM * HIDDEN_DIM, file); */
/*     read_count += fread(params->b2, sizeof(float), HIDDEN_DIM, file); */
/*     read_count += fread(params->w3, sizeof(float), HIDDEN_DIM * OUTPUT_DIM, file); */
/*     read_count += fread(params->b3, sizeof(float), OUTPUT_DIM, file); */

/*     if (read_count != expected) { */
/*         printf("Error: Expected %zu floats, read %zu\n", expected, read_count); */
/*         free(params); */
/*         fclose(file); */
/*         return NULL; */
/*     } */

/*     fclose(file); */
/*     return params; */
/* } */

/* void free_model_params(ModelParams* params) { */
/*     if (params) { */
/*         free(params); */
/*     } */
/* } */

/* float* generate_heightmap(ModelParams* params) { */
/*     printf("Starting heightmap generation...\n"); */
/*     printf("  GRID_SIZE: %d, TOTAL_SIZE: %d\n", GRID_SIZE, TOTAL_SIZE); */
/*     printf("  HIDDEN_DIM: %d, INPUT_DIM: %d, OUTPUT_DIM: %d\n", HIDDEN_DIM, INPUT_DIM, OUTPUT_DIM); */

/*     float* heightmap = (float*)malloc(TOTAL_SIZE * sizeof(float)); */
/*     if (!heightmap) { */
/*         printf("Error: Heightmap memory allocation failed\n"); */
/*         return NULL; */
/*     } */
/*     printf("  Allocated heightmap buffer: %d floats\n", TOTAL_SIZE); */

/*     float h1[HIDDEN_DIM]; */
/*     float h2[HIDDEN_DIM]; */

/*     // Log sample points (e.g., (0,0), (64,64), (127,127)) */
/*     int sample_coords[] = {0, 0, 64, 64, 127, 127}; */
/*     int num_samples = 3;  // 3 pairs of (x,z) */

/*     for (int z = 0; z < GRID_SIZE; z++) { */
/*         for (int x = 0; x < GRID_SIZE; x++) { */
/*             float u = (float)x / (GRID_SIZE - 1); */
/*             float v = (float)z / (GRID_SIZE - 1); */
/*             float coords[INPUT_DIM] = {u, v}; */

/*             // Log inputs for sample points */
/*             int log_sample = 0; */
/*             for (int s = 0; s < num_samples; s++) { */
/*                 if (x == sample_coords[s * 2] && z == sample_coords[s * 2 + 1]) { */
/*                     log_sample = 1; */
/*                     printf("Processing sample point (%d, %d): coords = [%f, %f]\n", x, z, u, v); */
/*                     break; */
/*                 } */
/*             } */

/*             // Layer 1: coords -> h1 */
/*             for (int i = 0; i < HIDDEN_DIM; i++) { */
/*                 h1[i] = params->b1[i]; */
/*                 for (int j = 0; j < INPUT_DIM; j++) { */
/*                     h1[i] += coords[j] * params->w1[j][i]; */
/*                 } */
/*                 h1[i] = tanh_approx(h1[i]); */
/*             } */
/*             if (log_sample) { */
/*                 printf("  Layer 1 output (h1): [%f, %f, %f, ...] (first 3 of %d)\n", */
/*                        h1[0], h1[1], h1[2], HIDDEN_DIM); */
/*             } */

/*             // Layer 2: h1 -> h2 */
/*             for (int i = 0; i < HIDDEN_DIM; i++) { */
/*                 h2[i] = params->b2[i]; */
/*                 for (int j = 0; j < HIDDEN_DIM; j++) { */
/*                     h2[i] += h1[j] * params->w2[j][i]; */
/*                 } */
/*                 h2[i] = tanh_approx(h2[i]);  // Fixed: was h1[i], should be h2[i] */
/*             } */
/*             if (log_sample) { */
/*                 printf("  Layer 2 output (h2): [%f, %f, %f, ...] (first 3 of %d)\n", */
/*                        h2[0], h2[1], h2[2], HIDDEN_DIM); */
/*             } */

/*             // Output layer: h2 -> out */
/*             float out = params->b3[0]; */
/*             for (int i = 0; i < HIDDEN_DIM; i++) { */
/*                 out += h2[i] * params->w3[i][0]; */
/*             } */
/*             heightmap[z * GRID_SIZE + x] = out * 5.0f; */

/*             if (log_sample) { */
/*                 printf("  Final height at (%d, %d): %f\n", x, z, heightmap[z * GRID_SIZE + x]); */
/*             } */
/*         } */
/*     } */

/*     // Log some stats */
/*     float min_height = heightmap[0]; */
/*     float max_height = heightmap[0]; */
/*     double sum = 0.0; */
/*     for (int i = 0; i < TOTAL_SIZE; i++) { */
/*         if (heightmap[i] < min_height) min_height = heightmap[i]; */
/*         if (heightmap[i] > max_height) max_height = heightmap[i]; */
/*         sum += heightmap[i]; */
/*     } */
/*     float mean_height = (float)(sum / TOTAL_SIZE); */
/*     printf("Heightmap generation completed.\n"); */
/*     printf("  Min height: %f\n", min_height); */
/*     printf("  Max height: %f\n", max_height); */
/*     printf("  Mean height: %f\n", mean_height); */

/*     return heightmap; */
/* } */

/* /\* int main() { *\/ */
/* /\*     printf("Starting heightmap generation test...\n"); *\/ */

/* /\*     // Load model parameters *\/ */
/* /\*     printf("Loading model parameters from 'model_weights.bin'...\n"); *\/ */
/* /\*     ModelParams* params = load_model_params("model_weights.bin"); *\/ */
/* /\*     if (!params) { *\/ */
/* /\*         printf("Failed to load model parameters. Exiting.\n"); *\/ */
/* /\*         return 1; *\/ */
/* /\*     } *\/ */
/* /\*     printf("Model parameters loaded successfully.\n"); *\/ */

/* /\*     // Log some parameter values to verify loading *\/ */
/* /\*     printf("Verifying loaded parameters:\n"); *\/ */
/* /\*     printf("  w1[0][0]: %f, w1[1][0]: %f\n", params->w1[0][0], params->w1[1][0]); *\/ */
/* /\*     printf("  b1[0]: %f, b1[1]: %f\n", params->b1[0], params->b1[1]); *\/ */
/* /\*     printf("  w2[0][0]: %f, w2[1][1]: %f\n", params->w2[0][0], params->w2[1][1]); *\/ */
/* /\*     printf("  b2[0]: %f, b2[1]: %f\n", params->b2[0], params->b2[1]); *\/ */
/* /\*     printf("  w3[0][0]: %f, w3[1][0]: %f\n", params->w3[0][0], params->w3[1][0]); *\/ */
/* /\*     printf("  b3[0]: %f\n", params->b3[0]); *\/ */

/* /\*     // Generate heightmap *\/ */
/* /\*     printf("Generating heightmap...\n"); *\/ */
/* /\*     float* heightmap = generate_heightmap(params); *\/ */
/* /\*     if (!heightmap) { *\/ */
/* /\*         printf("Failed to generate heightmap. Exiting.\n"); *\/ */
/* /\*         free_model_params(params); *\/ */
/* /\*         return 1; *\/ */
/* /\*     } *\/ */
/* /\*     printf("Heightmap generated successfully. Size: %d floats (%dx%d grid)\n",  *\/ */
/* /\*            TOTAL_SIZE, GRID_SIZE, GRID_SIZE); *\/ */

/* /\*     // Compute statistics *\/ */
/* /\*     float min_height = heightmap[0]; *\/ */
/* /\*     float max_height = heightmap[0]; *\/ */
/* /\*     double sum = 0.0;  // Use double for better precision in mean *\/ */
/* /\*     for (int i = 0; i < TOTAL_SIZE; i++) { *\/ */
/* /\*         if (heightmap[i] < min_height) min_height = heightmap[i]; *\/ */
/* /\*         if (heightmap[i] > max_height) max_height = heightmap[i]; *\/ */
/* /\*         sum += heightmap[i]; *\/ */
/* /\*     } *\/ */
/* /\*     float mean_height = (float)(sum / TOTAL_SIZE); *\/ */

/* /\*     // Print statistics *\/ */
/* /\*     printf("Heightmap statistics:\n"); *\/ */
/* /\*     printf("  Min height: %f\n", min_height); *\/ */
/* /\*     printf("  Max height: %f\n", max_height); *\/ */
/* /\*     printf("  Mean height: %f\n", mean_height); *\/ */

/* /\*     // Print sample values at key points *\/ */
/* /\*     printf("Sample heights at key points:\n"); *\/ */
/* /\*     printf("  Top-left (0,0): %f\n", heightmap[0]); *\/ */
/* /\*     printf("  Top-right (0,127): %f\n", heightmap[GRID_SIZE - 1]); *\/ */
/* /\*     printf("  Center (~64,64): %f\n", heightmap[(GRID_SIZE / 2) * GRID_SIZE + (GRID_SIZE / 2)]); *\/ */
/* /\*     printf("  Bottom-left (127,0): %f\n", heightmap[(GRID_SIZE - 1) * GRID_SIZE]); *\/ */
/* /\*     printf("  Bottom-right (127,127): %f\n", heightmap[TOTAL_SIZE - 1]); *\/ */

/* /\*     // Clean up *\/ */
/* /\*     printf("Cleaning up...\n"); *\/ */
/* /\*     free(heightmap); *\/ */
/* /\*     free_model_params(params); *\/ */
/* /\*     printf("Test completed successfully.\n"); *\/ */

/* /\*     return 0; *\/ */
/* /\* } *\/ */

/* float* load_heightmap_from_npy(const char* filename) { */
/*     FILE* file = fopen(filename, "rb"); */
/*     if (!file) { */
/*         printf("Error: Could not open .npy file %s: %s\n", filename, strerror(errno)); */
/*         return NULL; */
/*     } */

/*     // Read magic string "\x93NUMPY" */
/*     char magic[6]; */
/*     if (fread(magic, 1, 6, file) != 6 || memcmp(magic, "\x93NUMPY", 6) != 0) { */
/*         printf("Error: Not a valid .npy file\n"); */
/*         fclose(file); */
/*         return NULL; */
/*     } */

/*     // Read version (2 bytes) */
/*     uint8_t major, minor; */
/*     fread(&major, 1, 1, file); */
/*     fread(&minor, 1, 1, file); */
/*     printf("NPY version: %d.%d\n", major, minor); */

/*     // Read header length (4 bytes for v2+, 2 bytes for v1) */
/*     uint32_t header_len; */
/*     if (major == 1) { */
/*         uint16_t len16; */
/*         fread(&len16, sizeof(uint16_t), 1, file); */
/*         header_len = len16; */
/*     } else { */
/*         fread(&header_len, sizeof(uint32_t), 1, file); */
/*     } */
/*     printf("Header length: %u bytes\n", header_len); */

/*     // Read and print header for debugging */
/*     char* header = (char*)malloc(header_len + 1); */
/*     if (!header || fread(header, 1, header_len, file) != header_len) { */
/*         printf("Error: Failed to read header\n"); */
/*         free(header); */
/*         fclose(file); */
/*         return NULL; */
/*     } */
/*     header[header_len] = '\0'; // Null-terminate */
/*     printf("Header: %s\n", header); */
/*     free(header); */

/*     // Allocate buffer */
/*     float* heightmap = (float*)malloc(TOTAL_SIZE * sizeof(float)); */
/*     if (!heightmap) { */
/*         printf("Error: Memory allocation failed\n"); */
/*         fclose(file); */
/*         return NULL; */
/*     } */

/*     // Read data */
/*     size_t expected = TOTAL_SIZE; */
/*     size_t read_count = fread(heightmap, sizeof(float), TOTAL_SIZE, file); */
/*     if (read_count != expected) { */
/*         printf("Error: Expected %zu floats, read %zu\n", expected, read_count); */
/*         free(heightmap); */
/*         fclose(file); */
/*         return NULL; */
/*     } */

/*     fclose(file); */
/*     printf("Successfully loaded %zu floats\n", read_count); */
/*     return heightmap; */
/* } */


/* float* generate_new_heightmap(const char* weights_file) { */
/*     printf("Loading model parameters from %s...\n", weights_file); */
/*     ModelParams* params = load_model_params(weights_file); */
/*     if (!params) { */
/*         printf("Failed to load model parameters.\n"); */
/*         return NULL; */
/*     } */
/*     printf("Generating new heightmap...\n"); */
/*     float* heightmap = generate_heightmap(params); */
/*     free_model_params(params);  // Free params immediately after use */
/*     if (!heightmap) { */
/*         printf("Failed to generate heightmap.\n"); */
/*     } */
/*     return heightmap; */
/* } */

/* void update_terrain_heightmap(Renderable* renderable, const char* weights_file) { */

/*     float* new_heightmap = generate_new_heightmap(weights_file); */
/*     if (!new_heightmap) { */
/*         printf("Failed to update heightmap.\n"); */
/*         return; */
/*     } */

/*     float* vertices = (float*)malloc(TOTAL_SIZE * 6 * sizeof(float)); */
/*     if (!vertices) { */
/*         printf("Error: Memory allocation failed for vertices.\n"); */
/*         free(new_heightmap); */
/*         return; */
/*     } */
/*     int vertex_idx = 0; */
/*     for (int z = 0; z < GRID_SIZE; z++) { */
/*         for (int x = 0; x < GRID_SIZE; x++) { */
/*             float height = new_heightmap[z * GRID_SIZE + x]; */
/*             vertices[vertex_idx++] = (float)x; */
/*             vertices[vertex_idx++] = height; */
/*             vertices[vertex_idx++] = (float)z; */
/*             float normalized_height = (height + 5.0f) / 10.0f; */
/*             vertices[vertex_idx++] = normalized_height; */
/*             vertices[vertex_idx++] = 1.0f - normalized_height; */
/*             vertices[vertex_idx++] = 0.2f; */
/*         } */
/*     } */

/*     sg_update_buffer(renderable->mesh->vertex_buffer, &(sg_range){ */
/*         .ptr = vertices, */
/*         .size = TOTAL_SIZE * 6 * sizeof(float) */
/*     }); */
/*     printf("Terrain heightmap updated successfully.\n"); */

/*     free(vertices); */
/*     free(new_heightmap); */
/* } */
