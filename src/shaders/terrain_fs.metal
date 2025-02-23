#include <metal_stdlib>
using namespace metal;

struct fs_in {
    float3 color;
};

fragment float4 _main(fs_in in [[stage_in]]) {
    return float4(in.color, 1.0);
}