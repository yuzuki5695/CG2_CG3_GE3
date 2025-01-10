#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int endbleLighting;
    float32_t4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color; //!< ライトの色
    float3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};

struct Camera
{
    float3 worldPosition;
};

struct PointLight
{
    float4 color; //!< ライトの色
    float3 position; //!< ライトの位置
    float intensity; //!< 輝度
};


ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<PointLight> gPointLight : register(b3);

struct PixeShaderOutput
{
    float4 color : SV_TARGET0;
    
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixeShaderOutput main(VertexShaderOutput input)
{
    //TextureをSampling
    float4 TransformesUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, TransformesUV.xy);
    
    PixeShaderOutput output;
     
    // textureのα値が0.5以下のときにpixelを棄却
    if (textureColor.a <= 0.5)
    {
        discard;
    }

    // textureのα値が0のときにpixelを棄却
    if (textureColor.a == 0.0)
    {
        discard;
    }
    
    // output.colorのα値が0のときにpixelを棄却
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    if (gMaterial.endbleLighting != 0)
    { // Linhthingする場合
        // half lambert
        float Ndont = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(Ndont * 0.5f + 0.5f, 2.0f);
        float3 toEve = normalize(gCamera.worldPosition - input.worldPosition);
        
        //float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
        //float RdotE = dot(reflectLight, toEve);
        //float specularPow = pow(saturate(RdotE), gMaterial.shininess); // 反射強度
        
        // 鏡面反射（ハーフベクトル法）
        float3 halfVector = normalize(-gDirectionalLight.direction + toEve);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度 
        // 拡散反射（ディレクショナルライト）
        float3 diffuse =
        gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        // 鏡面反射（ディレクショナルライト）
        float3 specular =
        gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);    
        
        float3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);
        // 鏡面反射（ポイントライト）
        float3 pointLightHalfVector = normalize(-pointLightDirection + toEve);
        float pointLightNDotH = dot(normalize(input.normal), pointLightHalfVector);
        float pointLightSpecularPow = pow(saturate(pointLightNDotH), gMaterial.shininess);
        // 拡散反射（ポイントライト）
        float3 pointLightDiffuse =
        gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cos * gPointLight.intensity;
        // 鏡面反射（ポイントライト）
        float3 pointLightSpecular =
        gPointLight.color.rgb * gPointLight.intensity * pointLightSpecularPow * float3(1.0f, 1.0f, 1.0f);
         
        //（ディレクショナルライト）*（ポイントライト）
        output.color.rgb = diffuse + specular + pointLightDiffuse + pointLightSpecular;
        //output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    { // Linhthingしない場合、前回までと同じ演算
        output.color = gMaterial.color * textureColor;
    }
    return output;
}