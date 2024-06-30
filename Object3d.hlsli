
struct VertexShaderOutput
{
    float32_t4 Position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};


struct Material
{
    Vector4 color;
    int32_t endbleLighting;
};

struct DirectionalLight
{
    float32_t4 color; //!< ライトの色
    float32_t3 disrection; //!< ライトの向き
    float intensity; //!< 輝度
};