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
    float radius; //!< ライトの届く最大距離
    float decay; //!< 減衰率
};

struct SpotLight
{
    float4 color; //!< ライトの色
    float3 position; //!< ライトの位置
    float intensity; //!< 輝度
    float3 direction; //!< スポットライトの向き
    float radius; //!< ライトの届く最大距離
    float decay; //!< 減衰率
    float cosAngle; //!< スポットライトの余弦
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<PointLight> gPointLight : register(b3);
ConstantBuffer<SpotLight> gSpotLight : register(b4);

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
                
        // 拡散反射（ディレクショナルライト）
        float3 diffuse =
        gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        // 鏡面反射（ディレクショナルライト）
        float3 halfVector = normalize(-gDirectionalLight.direction + toEve);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
        float3 specular =
        gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        
        float3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);

        // 拡散反射（ポイントライト）
        float distance = length(gPointLight.position - input.worldPosition); // ポイントライトへの距離
        float factor = pow(saturate(-distance / gPointLight.radius + 1.0), gPointLight.decay); // 指数によるコントロール
        float3 pointLightDiffuse =
        gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cos * gPointLight.intensity * factor;
        // 鏡面反射（ポイントライト）
        float3 pointLightHalfVector = normalize(-pointLightDirection + toEve);
        float pointLightNDotH = dot(normalize(input.normal), pointLightHalfVector);
        float pointLightSpecularPow = pow(saturate(pointLightNDotH), gMaterial.shininess);
        float3 pointLightSpecular =
        gPointLight.color.rgb * gPointLight.intensity * pointLightSpecularPow * float3(1.0f, 1.0f, 1.0f) * factor;
         
        //// スポットライト
        //float3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSpotLight.position);
        //float cosAngle = dot(spotLightDirectionOnSurface, gSpotLight.direction);
        //float falloffFactor = saturate((cos - gSpotLight.cosAngle) / (1.0f - gSpotLight.cosAngle));
        
        
        //float diffuseIntensity = max(dot(normalize(input.normal), -spotLightDirectionOnSurface), 0.0f);
        
        //float3 spotLighthalfVector = normalize(-spotLightDirectionOnSurface + toEye);
        //float spotLightNDotH = dot(normalize(input.normal), spotLighthalfVector);
        //float spotLightspecularIntensity = pow(saturate(spotLightNDotH), gMaterial.shininess);
        
        //// 拡散反射（スポットライト）
        //float3 spotLightDiffuse =
        //gSpotLight.color.rgb * gSpotLight.intensity * diffuseIntensity * falloffFactor;
        
        //// 鏡面反射（スポットライト）
        //float3 spotLightspecular =
        //gSpotLight.color.rgb * gSpotLight.intensity * spotLightspecularIntensity * falloffFactor;
       
        
        
        // ライト
        output.color.rgb = diffuse + specular + pointLightDiffuse + pointLightSpecular;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    { // Linhthingしない場合、前回までと同じ演算
        output.color = gMaterial.color * textureColor;
    }
    return output;
}