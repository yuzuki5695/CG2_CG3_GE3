#include "Object3d.h"
#include "Object3dCommon.h"
#include<fstream>
#include<sstream>
#include <cassert>
#include "MatrixVector.h"
#include "TextureManager.h"

void Object3d::Initialize(Object3dCommon* object3dCommon) {
	// NULL検出
	assert(object3dCommon);
	// 引数で受け取ってメンバ変数に記録する
	this->object3dCommon = object3dCommon;
	// モデル読み込み
	modelDate = LoadObjFile("Resources", "plane.obj");

    // 頂点データの作成
    VertexDatacreation();
    // マテリアルの生成、初期化
    MaterialGenerate();
    // WVP,World用のリソースの生成、初期化
    TransformationMatrixGenerate();
    // 平行光源の生成,初期化
    DirectionalLightGenerate();

    // .objの参照しているテクスチャ読み込み
    TextureManager::GetInstance()->LoadTexture(modelDate.material.textureFilePath);
    // 読み込んだテクスチャの番号を取得
    modelDate.material.textureindex = TextureManager::GetInstance()->GetTextureindexByFilePath(modelDate.material.textureFilePath);
    // transform変数を作る
    transform = { {1.0f,1.0f,1.0f},{0.0f,3.0f,0.0f},{0.0f,-0.5f,0.0f} };  
    cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-700.0f} };
}

void Object3d::Update() {
    
     transform.rotate.y += 0.01f;

    Matrix4x4 worludMatrix = MatrixVector::MakeAftineMatrix(transform.scale, transform.rotate, transform.translate);
    Matrix4x4 cameraMatrix = MatrixVector::MakeAftineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
    Matrix4x4 viewMatrix = MatrixVector::Inverse(cameraMatrix);
    Matrix4x4 projectionMatrix = MatrixVector::MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
    Matrix4x4 worldViewProjectionMatrix = MatrixVector::Multiply(worludMatrix, MatrixVector::Multiply(viewMatrix, projectionMatrix));
    transformationMatrixData->World = worludMatrix;
    transformationMatrixData->WVP = worldViewProjectionMatrix;
}


void Object3d::Draw() {
    // VertexBufferViewの設定
    object3dCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    // マテリアルCBufferの場所を設定
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    // 座標変化行列CBufferの場所を設定
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
    //SRVのDescriptortableの先頭を設定。２はrootParameter[2]である。
    //SRVを切り替えて画像を変えるS
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelDate.material.textureindex));
    // 平行光源用のCBufferの場所を設定 
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

    // 描画！(今回は球)
    object3dCommon->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelDate.vertices.size()), 1, 0, 0);
}

void Object3d::VertexDatacreation() {

    // 関数化したResouceで作成
    vertexResoruce = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::VertexData) * modelDate.vertices.size());

    //頂点バッファビューを作成する
    // リソースの先頭のアドレスから使う
    vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    // 使用するリソースのサイズはの頂点のサイズ
    vertexBufferView.SizeInBytes = UINT(sizeof(Object3d::VertexData) * modelDate.vertices.size());
    // 1頂点当たりのサイズ
    vertexBufferView.StrideInBytes = sizeof(Object3d::VertexData);

    // 頂点リソースにデータを書き込むためのアドレスを取得
    vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    // 頂点データをリソースにコピー
    std::memcpy(vertexData, modelDate.vertices.data(), sizeof(Object3d::VertexData) * modelDate.vertices.size());
}

void Object3d::MaterialGenerate() {
    // マテリアル用のリソース
    materialResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::Material));
    // マテリアル用にデータを書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // マテリアルデータの初期値を書き込む
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->endbleLighting = true;
    materialData->uvTransform = MatrixVector::MakeIdentity4x4();
}

void Object3d::TransformationMatrixGenerate() {
    // WVP,World用のリソースを作る。TransformationMatrixを用意する
    transformationMatrixResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::TransformationMatrix));
    // データを書き込むためのアドレスを取得
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
    // 単位行列を書き込んでおく
    transformationMatrixData->WVP = MatrixVector::MakeIdentity4x4();
    transformationMatrixData->World = MatrixVector::MakeIdentity4x4();
}

void Object3d::DirectionalLightGenerate() {
    // 平行光源用のリソースを作る
    directionalLightResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::DirectionalLight));
    // 平行光源用にデータを書き込むためのアドレスを取得
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDate));
    // デフォルト値はとりあえず以下のようにして置く
    directionalLightDate->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightDate->direction = { 0.0f,-1.0f,0.0f };
    directionalLightDate->intensity = 1.0f;
}

Object3d::MaterialDate Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    Object3d::MaterialDate materialDate; // 構築するMaterialDate
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

Object3d::ModelDate Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    Object3d::ModelDate modelDate; // 構築するModelDate
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
            Object3d::VertexData triangle[3];
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