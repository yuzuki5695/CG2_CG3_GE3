
struct Material
{
    float32_t4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct PixeShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixeShaderOutput main()
{
    PixeShaderOutput output;
    output.color = gMaterial.color;
    return output;
}