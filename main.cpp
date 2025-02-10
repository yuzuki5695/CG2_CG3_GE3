#include<string>
#include<format>
#include<dxgi1_6.h>
#include<dxgidebug.h>
#include<dxcapi.h>
#include<cmath>
#include<assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include"MatrixVector.h"
#include<fstream>
#include<sstream>
#include"ResourceObject.h"
#include "Input.h"
#include "DirectXCommon.h"
#include"D3DResourceLeakChecker.h"
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"

struct Transform {
    Vector3 scale;
    Vector3 rotate;
    Vector3 translate;
};

struct VertexData
{
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};

struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
};

struct Material {
    Vector4 color;
    int32_t endbleLighting;
    float padding[3];
    Matrix4x4 uvTransform;
};

struct DirectionalLight {
    Vector4 color; //!< ライトの色
    Vector3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};

struct MaterialDate {
    std::string textureFilePath;
};



struct ModelDate {
    std::vector<VertexData> vertices;
    MaterialDate material;
};

/*----------------------------------------------------------------------*/
/*-------------------------Objファイルを読む関数---------------------------*/
/*----------------------------------------------------------------------*/


MaterialDate LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    MaterialDate materialDate; // 構築するMaterialDate
    std::string line; // ファイルから読んだ1行を格納するもの
    std::ifstream file(directoryPath + "/" + filename); // 2.ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める
    // 3. 実際にファイルを読み、MaterialDateを構築していく
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        // identifierの応じた処理
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            // 連結してファイルパスにする
            materialDate.textureFilePath = directoryPath + "/" + textureFilename;
        }
    }
    return materialDate;
}

ModelDate LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    ModelDate modelDate; // 構築するModelDate
    std::vector<Vector4> positions; // 位置
    std::vector<Vector3> normals; // 法線
    std::vector<Vector2> texcoords; // テクスチャ座標
    std::string line; // ファイルから読んだ1桁を格納するもの
    // 2.  ファイルを開く
    std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める

    // 3. 実際にファイルを読み、ModelDateを構築していく
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;// 先頭の識別子を読む

        // identifierの応じた処理
        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.x *= -1.0f;// 位置のx成分を反転
            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoord.y = 1.0f - texcoord.y;
            texcoords.push_back(texcoord);
        } else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.x *= -1.0f;// 法線のx成分を反転
            normals.push_back(normal);
        } else if (identifier == "f") {
            VertexData triangle[3];
            // 面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                // 頂点の要素へのIndexは、[位置/UV/法線]で格納されているので、分解してIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (uint32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/');// /区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }
                // 要素のIndexから、実際の要素の値を取得して、頂点を構築する
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];
                //VertexData vertex = { position,texcoord,normal };
                //modelDate.vertices.push_back(vertex);
                triangle[faceVertex] = { position,texcoord,normal };
            }
            // 頂点を逆順で登録することで、回り順を逆にする
            modelDate.vertices.push_back(triangle[2]);
            modelDate.vertices.push_back(triangle[1]);
            modelDate.vertices.push_back(triangle[0]);
        } else if (identifier == "mtllib") {
            // materialTemplateLibrarvファイルの名前を取得する
            std::string materialFilename;
            s >> materialFilename;
            // 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
            modelDate.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }
    // 4. ModelDateを返す
    return modelDate;
}

bool DepthFunc(float currZ, float prevZ) {
    return currZ <= prevZ;
}

/*------------------------------------------------------------------------------------*/
/*-------------------------------------球の作成関数-------------------------------------*/
/*------------------------------------------------------------------------------------*/

void DrawSphere(const uint32_t ksubdivision, VertexData* vertexdata) {
    // 球の頂点数を計算する
    //経度分割1つ分の角度 
    const float kLonEvery = (float)M_PI * 2.0f / float(ksubdivision);
    //緯度分割1つ分の角度 
    const float kLatEvery = (float)M_PI / float(ksubdivision);
    //経度の方向に分割
    for (uint32_t latIndex = 0; latIndex < ksubdivision; ++latIndex)
    {
        float lat = -(float)M_PI / 2.0f + kLatEvery * latIndex;	// θ
        //経度の方向に分割しながら線を描く
        for (uint32_t lonIndex = 0; lonIndex < ksubdivision; ++lonIndex)
        {
            float u = float(lonIndex) / float(ksubdivision);
            float v = 1.0f - float(latIndex) / float(ksubdivision);

            //頂点位置を計算する
            uint32_t start = (latIndex * ksubdivision + lonIndex) * 6;
            float lon = lonIndex * kLonEvery;	// Φ
            //頂点にデータを入力する。基準点 a
            vertexdata[start + 0].position = { cos(lat) * cos(lon) ,sin(lat) , cos(lat) * sin(lon) ,1.0f };
            vertexdata[start + 0].texcoord = { u,v };
            vertexdata[start + 0].normal.x = vertexdata[start + 0].position.x;
            vertexdata[start + 0].normal.y = vertexdata[start + 0].position.y;
            vertexdata[start + 0].normal.z = vertexdata[start + 0].position.z;

            //基準点 b
            vertexdata[start + 1].position = { cos(lat + kLatEvery) * cos(lon),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon) ,1.0f };
            vertexdata[start + 1].texcoord = { u ,v - 1.0f / float(ksubdivision) };
            vertexdata[start + 1].normal.x = vertexdata[start + 1].position.x;
            vertexdata[start + 1].normal.y = vertexdata[start + 1].position.y;
            vertexdata[start + 1].normal.z = vertexdata[start + 1].position.z;

            //基準点 c
            vertexdata[start + 2].position = { cos(lat) * cos(lon + kLonEvery),sin(lat), cos(lat) * sin(lon + kLonEvery) ,1.0f };
            vertexdata[start + 2].texcoord = { u + 1.0f / float(ksubdivision),v };
            vertexdata[start + 2].normal.x = vertexdata[start + 2].position.x;
            vertexdata[start + 2].normal.y = vertexdata[start + 2].position.y;
            vertexdata[start + 2].normal.z = vertexdata[start + 2].position.z;

            //基準点 d
            vertexdata[start + 3].position = { cos(lat + kLatEvery) * cos(lon + kLonEvery), sin(lat + kLatEvery) , cos(lat + kLatEvery) * sin(lon + kLonEvery) ,1.0f };
            vertexdata[start + 3].texcoord = { u + 1.0f / float(ksubdivision), v - 1.0f / float(ksubdivision) };
            vertexdata[start + 3].normal.x = vertexdata[start + 3].position.x;
            vertexdata[start + 3].normal.y = vertexdata[start + 3].position.y;
            vertexdata[start + 3].normal.z = vertexdata[start + 3].position.z;

            // 頂点4 (b, c, d)
            vertexdata[start + 4].position = { cos(lat) * cos(lon + kLonEvery),sin(lat),cos(lat) * sin(lon + kLonEvery),1.0f };
            vertexdata[start + 4].texcoord = { u + 1.0f / float(ksubdivision) ,v };
            vertexdata[start + 4].normal.x = vertexdata[start + 4].position.x;
            vertexdata[start + 4].normal.y = vertexdata[start + 4].position.y;
            vertexdata[start + 4].normal.z = vertexdata[start + 4].position.z;

            vertexdata[start + 5].position = { cos(lat + kLatEvery) * cos(lon),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon),1.0f };
            vertexdata[start + 5].texcoord = { u,v - 1.0f / float(ksubdivision) };
            vertexdata[start + 5].normal.x = vertexdata[start + 5].position.x;
            vertexdata[start + 5].normal.y = vertexdata[start + 5].position.y;
            vertexdata[start + 5].normal.z = vertexdata[start + 5].position.z;
        }
    }
}



//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    OutputDebugStringA("Hello,Directx!\n");
    
    HRESULT hr;
    
    // ポインタ
    Input* input = nullptr;
    WinApp* winApp = nullptr;
    DirectXCommon* dxCommon = nullptr;

    // ウィンドウ作成
    
    // WindowsAPIの初期化
    winApp = new WinApp();
    winApp->Initialize();


    // DirectXの初期化
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);


    // 汎用機能の初期化 



    // 入力の初期化
    input = new Input();
    input->Initialize(winApp);



    // シーンの初期化

    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;

    //DescriptorRange作成
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[4] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[0].Descriptor.ShaderRegister = 0;// レジスタ番号0を使う

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;// VertexShaderでを使う
    rootParameters[1].Descriptor.ShaderRegister = 0;// レジスタ番号0を使う

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//利用する数

    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[3].Descriptor.ShaderRegister = 1;// レジスタ番号1を使う

    //RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.pParameters = rootParameters;// ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = _countof(rootParameters);// 配列の長さ

    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ	
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0～1の範囲外をリピート		
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない		
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う	
    staticSamplers[0].ShaderRegister = 0;//レジスタ番号０を使う
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


    /*------------------------------------------------------------------------------------*/
    /*----------------------------------Resourceの作成-------------------------------------*/
    /*------------------------------------------------------------------------------------*/

    /*------------------------------------------------------------------*/
    /*----------------------マテリアル用のResource------------------------*/
    /*------------------------------------------------------------------*/

    // マテリアル用のリソース
    Microsoft::WRL::ComPtr <ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
    // マテリアル用にデータを書き込む
    Material* materialData = nullptr;
    // 書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // 今回は白
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    // Lightingするのでtrueを設定する
    materialData->endbleLighting = true;
    // 単位行列を書き込んでおく
    materialData->uvTransform = MakeIdentity4x4();

    /*------------------------------------------------------------------*/
    /*----------------TransformationMatrix用のResource-------------------*/
    /*------------------------------------------------------------------*/

     // WVP,World用のリソースを作る。TransformationMatrixを用意する
    Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
    // データを書き込む
    TransformationMatrix* transformationMatrixData = nullptr;
    // 書き込むためのアドレスを取得
    wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
    // 単位行列を書き込んでおく
    transformationMatrixData->WVP = MakeIdentity4x4();
    transformationMatrixData->World = MakeIdentity4x4();

    /*------------------------------------------------------------------*/
    /*------------------------Sprite用のResource-------------------------*/
    /*------------------------------------------------------------------*/

     //Sprite用のマテリアルリソースを作る
    Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));
    // Sprite用にデータを書き込む
    Material* materialSpriteDate = nullptr;
    // 書き込むためのアドレスを取得
    materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialSpriteDate));
    // 今回は白
    materialSpriteDate->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    // SpriteはLightingしないでfalseを設定する
    materialSpriteDate->endbleLighting = false;
    // 単位行列を書き込んでおく
    materialSpriteDate->uvTransform = MakeIdentity4x4();

    /*------------------------------------------------------------------*/
    /*-----------------------平行光源用のResource-------------------------*/
    /*------------------------------------------------------------------*/

     // 平行光源用のリソースを作る
    Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
    // 平行光源用にデータを書き込む
    DirectionalLight* directionalLightDate = nullptr;
    // 書き込むためのアドレスを取得
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDate));
    // デフォルト値はとりあえず以下のようにして置く
    directionalLightDate->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightDate->direction = { 0.0f,-1.0f,0.0f };
    directionalLightDate->intensity = 1.0f;

    /*-------------------------------------------------------*/
/*----------------------球のデータ-------------------------*/
/*-------------------------------------------------------*/

    const uint32_t kSubdivision = 16; //球の分割数

    uint32_t vertexCount = kSubdivision * kSubdivision * 6; //球の頂点数

    // モデル読み込み(axis or plane)
    ModelDate modelDate = LoadObjFile("Resources", "plane.obj");

    // 関数化したResouceで作成
    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce = dxCommon->CreateBufferResource(sizeof(VertexData) * modelDate.vertices.size());

    // 関数化したResouceで作成
    // Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce = CreateBufferResource(device, sizeof(VertexData) * vertexCount);

    //頂点バッファビューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    // リソースの先頭のアドレスから使う
    vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    // 使用するリソースのサイズはの頂点のサイズ
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelDate.vertices.size());

    //vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;

    // 1頂点当たりのサイズ
    vertexBufferView.StrideInBytes = sizeof(VertexData);

    //頂点リソースにデータを書き込む
    VertexData* vertexData = nullptr;
    //書き込むためのアドレスを取得
    vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    // 頂点データをリソースにコピー
    std::memcpy(vertexData, modelDate.vertices.data(), sizeof(VertexData) * modelDate.vertices.size());

    //// 球の頂点にデータを入力
    //DrawSphere(kSubdivision, vertexData);

    /*-------------------------------------------------------*/
    /*----------------------spriteのデータ---------------------*/
    /*------------------------------------------------------*/

    // Sprite用の頂点リソースを作る
    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);

    //頂点バッファビューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
    // リソースの先頭のアドレスから使う
    vertexBufferViewSprite.BufferLocation = vertexResoruceSprite->GetGPUVirtualAddress();
    // 使用するリソースのサイズは4つ分のサイズ
    vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
    // 1頂点当たりのサイズ
    vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

    //頂点リソースにデータを書き込む
    VertexData* vertexDataSprite = nullptr;
    //書き込むためのアドレスを取得
    vertexResoruceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

    vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
    vertexDataSprite[0].texcoord = { 0.0f,1.0f };
    vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
    vertexDataSprite[1].texcoord = { 0.0f,0.0f };
    vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
    vertexDataSprite[2].texcoord = { 1.0f,1.0f };
    vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };
    vertexDataSprite[3].texcoord = { 1.0f,0.0f };
    for (int i = 0; i < 4; i++) {
        vertexDataSprite[i].normal = { 0.0f,0.0f,-1.0f };
    }

    Microsoft::WRL::ComPtr <ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
    //頂点バッファビューを作成する
    D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
    // リソースの先頭のアドレスから使う
    indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
    // 使用するリソースのサイズは6つ分のサイズ
    indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
    // インデックスはuint32_tとする
    indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
    // インデックスリソースにデータを書き込む
    uint32_t* indexDateSprite = nullptr;
    indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDateSprite));
    indexDateSprite[0] = 0; indexDateSprite[1] = 1; indexDateSprite[2] = 2;
    indexDateSprite[3] = 1; indexDateSprite[4] = 3; indexDateSprite[5] = 2;

    // Sprite用のTransformationMatrix用のリソースを作る。
    Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
    // データを書き込む
    TransformationMatrix* transformationMatrixDateSprite = nullptr;
    // 書き込むためのアドレスを取得
    transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDateSprite));
    // 単位行列を書き込んでおく
    transformationMatrixDateSprite->World = MakeIdentity4x4();
    transformationMatrixDateSprite->WVP = MakeIdentity4x4();

    /*-----------------------------------------------------------------------------------*/
    /*--------------------------------Resourceの作成終了-----------------------------------*/
    /*-----------------------------------------------------------------------------------*/

     //シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    //バイナリを元に作成
    Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
    hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    //======== InputLayout設定 ==========//
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    //======= BlendStateの設定 =========//
    D3D12_BLEND_DESC blendDesc{};
    //全ての色要素を書き込む
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    //===== RasterizerStateの設定を行う ======//   
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    //裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    //三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


    /*-------------------------------------------------------------------------------*/
    /*----------------------------各Descriptorの設定----------------------------------*/
    /*-------------------------------------------------------------------------------*/



    /*------------------------------------------------------------*/
    /*--------------------------SRVの設定--------------------------*/
    /*------------------------------------------------------------*/

    //Textureを読んで転送する
    DirectX::ScratchImage mipImages = dxCommon->LoadTexture("Resources/uvChecker.png");
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    Microsoft::WRL::ComPtr <ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
    dxCommon->UploadTextureData(textureResource, mipImages);

    //2枚目のTextureを読んで転送する
    //DirectX::ScratchImage mipImages2 = LoadTexture("resources/monsterBall.png");

    DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelDate.material.textureFilePath);
    const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
    Microsoft::WRL::ComPtr <ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2);
    dxCommon->UploadTextureData(textureResource2, mipImages2);


    //metaDataを基にSRVの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;				//2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
    srvDesc2.Format = metadata2.format;
    srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;				//2Dテクスチャ
    srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

    //SRVを作成するDescriptorHeapの場所を決める
    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);
    //先頭はImGuiが使っているのでその次を使う
    textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    //SRVの生成
    dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

    /*----------------------------------------------------------------------------------*/
    /*----------------------------各Descriptorの設定終了----------------------------------*/
    /*----------------------------------------------------------------------------------*/

     //======== ShaderをCompile ===========// 
    Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"Resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"Resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    //========== PSO生成 =============//
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();   // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;    // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };  // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };    // PixelShader
    graphicsPipelineStateDesc.BlendState = blendDesc;// BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; //RasterizerState
    // 書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    //利用するトポロジ(形状)のタイプ
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // どのように画面に色を打ち込むかの設定(気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = dxCommon->GetdepthStencilDesc();
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 実際に生成
    Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
    hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
    assert(SUCCEEDED(hr));


    Transform transform{ {1.0f,1.0f,1.0f},{0.0f,3.0f,0.0f},{0.0f,0.0f,0.0f} };

    Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

    Transform  cameratransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-500.0f} };

    Transform  uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };


    // ウィンドウの×ボタンが押されるまでループ
    while (true) {
        // Windowのメッセージ処理
        if (winApp->ProcessMessage()) {
            // ゲームループを抜ける
            break;
        }
        // ゲームの処理

        // 入力の更新
        input->Update();

        // 0を押している間true
        if (input->Pushkey(DIK_0)) {
            OutputDebugStringA("Hit 0 \n");
        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
        ImGui::ShowDemoWindow();

        ImGui::Begin("Sprite");
        ImGui::DragFloat3("scale", &transform.scale.x, 0.01f);
        ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f);
        ImGui::DragFloat3("translate", &transform.translate.x, 0.01f);
        ImGui::ColorEdit3("colorSprite", reinterpret_cast<float*>(materialSpriteDate));
        ImGui::DragFloat3("LightDirection", &directionalLightDate->direction.x, 0.01f);
        ImGui::DragFloat("LightIntensity", &directionalLightDate->intensity, 0.01f);
        ImGui::DragFloat3("SpriteTranslate", (&transformSprite.translate.x));
        ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
        ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
        ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
        ImGui::End();

        //ImGuiの描画コマンドを生成
        ImGui::Render();

        /*------------------------------------------*/
        /*---------MVP,WorldMatrixの行列を作る--------*/
        /*------------------------------------------*/

       // transform.rotate.y += 0.01f;

        Matrix4x4 worludMatrix = MakeAftineMatrix(transform.scale, transform.rotate, transform.translate);
        Matrix4x4 cameraMatrix = MakeAftineMatrix(cameratransform.scale, cameratransform.rotate, cameratransform.translate);
        Matrix4x4 viewMatrix = Inverse(cameraMatrix);
        Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
        Matrix4x4 worldViewProjectionMatrix = Multiply(worludMatrix, Multiply(viewMatrix, projectionMatrix));
        transformationMatrixData->World = worludMatrix;
        transformationMatrixData->WVP = worldViewProjectionMatrix;

        /*-------------------------------------------*/
        /*---Sprite用のWrldViewProjectionMatrixを作る---*/
        /*--------------------------------------------*/

        Matrix4x4 worludMatrixSprite = MakeAftineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
        Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
        Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
        Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worludMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
        transformationMatrixDateSprite->World = worludMatrixSprite;
        transformationMatrixDateSprite->WVP = worldViewProjectionMatrixSprite;

        /*----------------------------------------*/
        /*---------UVTransform用の行列を作る--------*/
        /*----------------------------------------*/

        Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
        uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
        uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
        materialSpriteDate->uvTransform = uvTransformMatrix;

        // 描画用のDescriptorHeapの設定
        ID3D12DescriptorHeap* descriptorHeap[] = { dxCommon->GetsrvDescriptorHeap().Get()};
        dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeap);


        // 描画前処理
        dxCommon->PreDraw();

        // RootSignatureを設定。PSOに設定しているけど別途設定が必要
        dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
        dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
        dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
        // 形状を設定。PSOに設定しているものとはまた別。同じものを設定する
        dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        // マテリアルCBufferの場所を設定
        dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
        // wvp用のCBufferの場所を設定
        dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
        //SRVのDescriptortableの先頭を設定。２はrootParameter[2]である。
        //SRVを切り替えて画像を変えるS
        dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
        // 平行光源用のCBufferの場所を設定 
        dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

        // 描画！(今回は球) 
        //  dxCommon->GetCommandList()->DrawInstanced(vertexCount, 1, 0, 0);

        // 描画！(今回は球) 
        dxCommon->GetCommandList()->DrawInstanced(UINT(modelDate.vertices.size()), 1, 0, 0);


        /*---------------------------------------------------*/
        /*-------------------2dの描画コマンド開始---------------*/
        /*---------------------------------------------------*/

         // Spriteの描画は常にuvCheckerにする
        dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

        dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);//IBVを設定
        // wvp用のCBufferの場所を設定
        dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
        // Spriteの描画。変更が必要なものだけ変更する
        dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
        // TransformationMatrixBufferの場所を設定
        dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
        // 描画! (DrawCall/ドローコール) 6個のインデックスを使用し1つのインスタンスを描画、その他は当面０で良い
        dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

        /*---------------------------------------------------*/
        /*-------------------2dの描画コマンド終了---------------*/
        /*---------------------------------------------------*/

       //実際のcommandListのImGuiの描画コマンドを積む
       ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList().Get());

        // 描画後処理
        dxCommon->PostDrow();
    }


    // ComPtrを扱っていないものの解放処理
    mipImages.Release();
    mipImages2.Release();

    // シーンの解放


    // 汎用機能の解放


    // 入力解放
    delete input;

    // DirectXの解放
    delete dxCommon;

    // ウィンドウ解放 
    // WindowsAPIの終了処理
    winApp->Finalize();
    // WindowsAPIの解放
    delete winApp;
    
    // ImGuiの終了処理。
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    return 0;
}