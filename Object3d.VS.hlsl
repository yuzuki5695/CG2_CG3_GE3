float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}


struct VertexShaderOutput{
float32_t4 Position : SV_POSILION;
};

struct VertexShaderInput{
float32_t4 Position : POSILION0;
};


VertexShaderOutput main(VertexShaderInput inPut){
VertexShaderOutput output;
output.Position =input.Position;
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