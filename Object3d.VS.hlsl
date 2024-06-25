#include "Object3d.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
};

ConstantBuffer<TransformationMatrix> gtransformationMatrix : register(b0);
struct VertexShaderInput
{
    float32_t4 Position : POSITION0;  
    float32_t2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderOutput input)
{
    VertexShaderOutput output;
    output.Position = mul(input.Position, gtransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    return output;
}