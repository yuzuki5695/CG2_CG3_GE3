#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t endbleLighting;
};

struct DirectionalLight
{
    float32_t4 color; //!< ライトの色
    float32_t3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct PixeShaderOutput
{
    float32_t4 color : SV_TARGET0;
    
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixeShaderOutput main(VertexShaderOutput input)
{
    //TextureをSampling
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    PixeShaderOutput output;
    
    if (gMaterial.endbleLighting != 0){ // Linhthingする場合
        float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.disrection));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    else{ // Linhthingしない場合、前回までと同じ演算
        output.color = gMaterial.color * textureColor;
    }
    return output;
}