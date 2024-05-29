
struct TransformationMatrix{
    float32_t4x4 WVP; 
};

ConstantBuffer<TransformationMatrix> gtransformationMatrix : register(b0);
struct VertexShaderOutput
{
    float32_t4 Position : SV_POSITION;
};

struct VertexShaderInput
{
    float32_t4 Position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.Position = mul(input.Position,gtransformationMatrix.WVP);
    return output;
}