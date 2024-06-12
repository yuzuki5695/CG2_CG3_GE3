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
    PixeShaderOutput output;
    output.color = gMaterial.color;
    return output;
}