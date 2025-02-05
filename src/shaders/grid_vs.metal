#include <metal_stdlib>
using namespace metal;

struct params_t {
    float4x4 mvp;
};

struct vs_in {
    float3 pos [[attribute(0)]];
    float3 color [[attribute(1)]];
};

struct vs_out {
    float4 pos [[position]];
    float3 color;
};

vertex vs_out _main(vs_in in [[stage_in]], constant params_t& params [[buffer(0)]]) {
    vs_out out;
    out.pos = params.mvp * float4(in.pos, 1.0);
    out.color = in.color;
    return out;
}
