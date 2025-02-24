/* #pragma once */

/* #include "types.h" */

#define GRID_SIZE 128
#define TOTAL_SIZE (GRID_SIZE * GRID_SIZE)
#define INPUT_DIM 2
#define HIDDEN_DIM 64
#define OUTPUT_DIM 1

/* // Model parameters structure */
/* typedef struct { */
/*     float w1[INPUT_DIM][HIDDEN_DIM];  // First layer weights */
/*     float b1[HIDDEN_DIM];            // First layer biases */
/*     float w2[HIDDEN_DIM][HIDDEN_DIM]; // Second layer weights */
/*     float b2[HIDDEN_DIM];            // Second layer biases */
/*     float w3[HIDDEN_DIM][OUTPUT_DIM]; // Output layer weights */
/*     float b3[OUTPUT_DIM];            // Output layer biases */
/* } ModelParams; */

/* // Function declarations */
/* ModelParams* load_model_params(const char* filename); */
/* void free_model_params(ModelParams* params); */
/* float* generate_heightmap(ModelParams* params); */
/* float tanh_approx(float x);  // Approximation of tanh for activation */
/* float* generate_new_heightmap(const char* weights_file); */
/* void update_terrain_heightmap(Renderable* renderable, const char* weights_file); */
/* float* load_heightmap_from_npy(const char* filename); */
