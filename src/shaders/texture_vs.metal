#include <metal_stdlib>
using namespace metal;

struct vs_in {
    float3 pos [[attribute(0)]];
    float2 uv  [[attribute(1)]];
};

struct vs_out {
    float4 pos [[position]];
    float2 uv;
};

vertex vs_out vs_main(vs_in in [[stage_in]], constant float4x4 &mvp [[buffer(0)]]) {
    vs_out out;
    out.pos = mvp * float4(in.pos, 1.0);
    out.uv = in.uv;
    return out;
}
