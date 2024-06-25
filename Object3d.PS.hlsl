#include "Object3d.hlsli"


cbuffer MaterialBuffer
{
    float32_t4 color;
}

ConstantBuffer<MaterialBuffer> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixeShaderOutput
{ 
    
    float32_t4 color : SV_TARGET0;
};

PixeShaderOutput main(VertexShaderOutput input)
{
    
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    PixeShaderOutput output;
    output.color = gMaterial.color * textureColor;
    return output;
}