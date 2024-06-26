#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct PixeShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixeShaderOutput main(VertexShaderOutput input)
{
    //TextureをSampling
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    PixeShaderOutput output;
    output.color = gMaterial.color;
    return output;
}

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);