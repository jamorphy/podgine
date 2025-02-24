// compute_heightmap.metal
#include <metal_stdlib>
using namespace metal;

kernel void heightmap_mlp(
    device float* coords [[buffer(0)]],   // 128x128 x [u, v]
    device float* w1 [[buffer(1)]],       // 2x64 weights
    device float* b1 [[buffer(2)]],       
    device float* w2 [[buffer(3)]],       // 64x64 weights
    device float* b2 [[buffer(4)]],       
    device float* w3 [[buffer(5)]],       // 64x1 weights
    device float* b3 [[buffer(6)]],       
    device float* output [[buffer(7)]],   // 128x128
    uint2 gid [[thread_position_in_grid]] // x, z
) {
    uint idx = gid.y * 128 + gid.x;
    float u = coords[idx * 2];
    float v = coords[idx * 2 + 1];

    float h1[64];
    for (uint i = 0; i < 64; i++) {
        h1[i] = b1[i];
        h1[i] += u * w1[i] + v * w1[64 + i];
        h1[i] = tanh(h1[i]);
    }

    float h2[64];
    for (uint i = 0; i < 64; i++) {
        h2[i] = b2[i];
        for (uint j = 0; j < 64; j++) {
            h2[i] += h1[j] * w2[j * 64 + i];
        }
        h2[i] = tanh(h1[i]);
    }

    float out = b3[0];
    for (uint i = 0; i < 64; i++) {
        out += h2[i] * w3[i];
    }

    output[idx] = out * 5.0f; // Scale for testing
}