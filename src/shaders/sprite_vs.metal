#include <metal_stdlib>
using namespace metal;

struct params_t {
    float4x4 mvp;
};

struct vs_in {
    float3 pos [[attribute(0)]];
    float2 texcoord [[attribute(1)]];
};

struct vs_out {
    float4 pos [[position]];
    float2 texcoord [[user(texcoord)]];  // Add the user attribute qualifier
};

vertex vs_out _main(vs_in in [[stage_in]], constant params_t& params [[buffer(0)]]) {
    vs_out out;
    out.pos = params.mvp * float4(in.pos, 1.0);
    out.texcoord = in.texcoord;
    return out;
}