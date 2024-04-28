


struct TransfomationMatrix {

	float32_t4x4 MVP;

};


ConstantBuffer<TransfomationMatrix> gtransformarionMatrix : registere(bo);

struct VertexShaderOutput{
float32_t4 position : SV_POSITION;
}



struct VertexShaderInput{
float32_t4 position : SV_POSITION;
}

VertexShaderOutput main(VertexShaderInput niput){


VertexShaderOutput output;

output.position =mul(input.position,ggtransformarionMatrix.MVP);

returbn potput;

};




struct PixelShaderOutput{

float32_t4 color : SV_TARGETO;

};
PixelShaderOutput main(){
PixelShaderOutput output;
output.color = float32_t4(1.0,1.0,1.0,1.0);
return output;

}

struct Material{
float32_t4 color;
};

ConstantBuffer<Naterial>gMaterial : register(b0);

struct PixeShaderOutput{
 float32_t4 color : SV_TARGET0;
};

PixeShaderOutput main(){
PixeShaderOutput output;
output.color = gMaterial.color;
return output; 
}