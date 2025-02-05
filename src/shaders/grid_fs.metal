#include <metal_stdlib>
using namespace metal;

fragment float4 _main(float3 color [[stage_in]]) {
    return float4(color, 1.0);
}
