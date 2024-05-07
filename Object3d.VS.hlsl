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


struct PixelShaderOutput{

float32_t4 color : SV_TARGETO;

};
PixelShaderOutput main(){
PixelShaderOutput output;
output.color = float32_t4(1.0,1.0,1.0,1.0);
return output;

}