#include <metal_stdlib>
using namespace metal;
fragment float4 fs_main(float2 uv [[stage_in]],
texture2d<float> tex [[texture(0)]],
sampler smp [[sampler(0)]]) {
return tex.sample(smp, uv);
}