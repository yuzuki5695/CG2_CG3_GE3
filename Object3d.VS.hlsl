struct VertexShaderOutput{
float32_t4 Position : SV_POSITION;
};

struct VertexShaderInput{
float32_t4 Position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input){
VertexShaderOutput output;
output.Position =input.position;
return output;
}