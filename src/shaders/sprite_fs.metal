#include <metal_stdlib>
using namespace metal;

struct fs_in {
    float2 texcoord [[user(texcoord)]];  // Add the user attribute qualifier
};

fragment float4 _main(fs_in in [[stage_in]],
                     texture2d<float> tex [[texture(0)]],
                     sampler smp [[sampler(0)]]) {
    return tex.sample(smp, in.texcoord);
}