typedef float4 float32_t4;
typedef float4 float32_t4*4;

struct TransformationMatrix{
    float32_t4x4 WVP; 
};





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
    output.Position = input.Position;
    return output;
}