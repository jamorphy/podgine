#include <Metal/Metal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> // For srand/rand

#include "utils.h"

// Hardcode dimensions
#define GRID_SIZE 128
#define TOTAL_SIZE (GRID_SIZE * GRID_SIZE)
#define INPUT_DIM 2
#define HIDDEN_DIM 64
#define OUTPUT_DIM 1

// Helper to check Metal errors
static void check_error(NSError* error, const char* step) {
    if (error) {
        printf("Error at %s: %s\n", step, [[error localizedDescription] UTF8String]);
        exit(1);
    }
}

// Dummy weights with more variance
static float w1[INPUT_DIM * HIDDEN_DIM];
static float b1[HIDDEN_DIM];
static float w2[HIDDEN_DIM * HIDDEN_DIM];
static float b2[HIDDEN_DIM];
static float w3[HIDDEN_DIM * OUTPUT_DIM];
static float b3[OUTPUT_DIM];

// Fill dummy arrays with wider range
static void init_dummy_weights(void) {
    srand((unsigned int)time(NULL)); // Seed for randomness
    for (int i = 0; i < INPUT_DIM * HIDDEN_DIM; i++) {
        w1[i] = ((float)rand() / RAND_MAX - 0.5f) * 10.0f; // -5 to 5 range
    }
    for (int i = 0; i < HIDDEN_DIM; i++) {
        b1[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;  // -1 to 1 range
    }
    for (int i = 0; i < HIDDEN_DIM * HIDDEN_DIM; i++) {
        w2[i] = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;  // -2 to 2 range
    }
    for (int i = 0; i < HIDDEN_DIM; i++) {
        b2[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;  // -1 to 1 range
    }
    for (int i = 0; i < HIDDEN_DIM * OUTPUT_DIM; i++) {
        w3[i] = ((float)rand() / RAND_MAX - 0.5f) * 6.0f;  // -3 to 3 range
    }
    b3[0] = ((float)rand() / RAND_MAX - 0.5f) * 1.0f;      // -0.5 to 0.5 range
}

float* generate_heightmap(void) {
    printf("Starting Metal heightmap generation...\n");
    printf("  GRID_SIZE: %d, TOTAL_SIZE: %d\n", GRID_SIZE, TOTAL_SIZE);

    // Allocate host memory
    float* heightmap = (float*)malloc(TOTAL_SIZE * sizeof(float));
    if (!heightmap) {
        printf("Error: Heightmap memory allocation failed\n");
        return NULL;
    }
    printf("  Allocated heightmap buffer: %d floats\n", TOTAL_SIZE);

    // Initialize dummy weights
    init_dummy_weights();

    // Create Metal device
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        printf("Failed to create Metal device.\n");
        free(heightmap);
        return NULL;
    }

    // Create command queue
    id<MTLCommandQueue> queue = [device newCommandQueue];
    if (!queue) {
        printf("Failed to create command queue.\n");
        free(heightmap);
        return NULL;
    }

    // Shader source
    const char* source = read_text_file("src/shaders/compute_heightmap.metal");
    NSString* sourceString = [NSString stringWithUTF8String:source];

    // Create shader library
    NSError* error = nil;
    id<MTLLibrary> library = [device newLibraryWithSource:sourceString options:nil error:&error];
    check_error(error, "library creation");

    // Create compute function
    id<MTLFunction> function = [library newFunctionWithName:@"heightmap_mlp"];

    // Create compute pipeline
    id<MTLComputePipelineState> pipeline = [device newComputePipelineStateWithFunction:function error:&error];
    check_error(error, "pipeline creation");

    // Generate input coords
    float* coords = (float*)malloc(TOTAL_SIZE * INPUT_DIM * sizeof(float));
    for (int z = 0; z < GRID_SIZE; z++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int idx = z * GRID_SIZE + x;
            coords[idx * 2] = (float)x / (GRID_SIZE - 1);     // u
            coords[idx * 2 + 1] = (float)z / (GRID_SIZE - 1); // v
        }
    }

    // Create buffers
    id<MTLBuffer> coords_buf = [device newBufferWithBytes:coords length:TOTAL_SIZE * INPUT_DIM * sizeof(float) options:MTLResourceStorageModeShared];
    id<MTLBuffer> w1_buf = [device newBufferWithBytes:w1 length:INPUT_DIM * HIDDEN_DIM * sizeof(float) options:MTLResourceStorageModeShared];
    id<MTLBuffer> b1_buf = [device newBufferWithBytes:b1 length:HIDDEN_DIM * sizeof(float) options:MTLResourceStorageModeShared];
    id<MTLBuffer> w2_buf = [device newBufferWithBytes:w2 length:HIDDEN_DIM * HIDDEN_DIM * sizeof(float) options:MTLResourceStorageModeShared];
    id<MTLBuffer> b2_buf = [device newBufferWithBytes:b2 length:HIDDEN_DIM * sizeof(float) options:MTLResourceStorageModeShared];
    id<MTLBuffer> w3_buf = [device newBufferWithBytes:w3 length:HIDDEN_DIM * OUTPUT_DIM * sizeof(float) options:MTLResourceStorageModeShared];
    id<MTLBuffer> b3_buf = [device newBufferWithBytes:b3 length:OUTPUT_DIM * sizeof(float) options:MTLResourceStorageModeShared];
    id<MTLBuffer> output_buf = [device newBufferWithLength:TOTAL_SIZE * sizeof(float) options:MTLResourceStorageModeShared];

    if (!coords_buf || !w1_buf || !b1_buf || !w2_buf || !b2_buf || !w3_buf || !b3_buf || !output_buf) {
        printf("Failed to create buffers.\n");
        free(coords);
        free(heightmap);
        return NULL;
    }
    free(coords);

    // Dispatch compute
    id<MTLCommandBuffer> cmd_buffer = [queue commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [cmd_buffer computeCommandEncoder];
    [encoder setComputePipelineState:pipeline];
    [encoder setBuffer:coords_buf offset:0 atIndex:0];
    [encoder setBuffer:w1_buf offset:0 atIndex:1];
    [encoder setBuffer:b1_buf offset:0 atIndex:2];
    [encoder setBuffer:w2_buf offset:0 atIndex:3];
    [encoder setBuffer:b2_buf offset:0 atIndex:4];
    [encoder setBuffer:w3_buf offset:0 atIndex:5];
    [encoder setBuffer:b3_buf offset:0 atIndex:6];
    [encoder setBuffer:output_buf offset:0 atIndex:7];

    MTLSize threadsPerGrid = MTLSizeMake(GRID_SIZE, GRID_SIZE, 1);
    MTLSize threadsPerGroup = MTLSizeMake(8, 8, 1);
    [encoder dispatchThreads:threadsPerGrid threadsPerThreadgroup:threadsPerGroup];
    [encoder endEncoding];
    [cmd_buffer commit];
    [cmd_buffer waitUntilCompleted];
    printf("Compute dispatched.\n");

    // Copy to host
    float* gpu_output = (float*)[output_buf contents];
    memcpy(heightmap, gpu_output, TOTAL_SIZE * sizeof(float));

    // Stats
    float min_height = heightmap[0];
    float max_height = heightmap[0];
    double sum = 0.0;
    for (int i = 0; i < TOTAL_SIZE; i++) {
        if (heightmap[i] < min_height) min_height = heightmap[i];
        if (heightmap[i] > max_height) max_height = heightmap[i];
        sum += heightmap[i];
    }
    float mean_height = (float)(sum / TOTAL_SIZE);
    printf("Heightmap generation completed.\n");
    printf("  Min height: %f\n", min_height);
    printf("  Max height: %f\n", max_height);
    printf("  Mean height: %f\n", mean_height);

    return heightmap;
}
