#include "ParticleManager.h"
#include <MatrixVector.h>
#include <cassert>
#include <ModelManager.h>
#include <TextureManager.h>
#include <numbers>
#include<GraphicsPipeline.h>
#include <iostream>

using namespace MatrixVector;
using namespace Microsoft::WRL;

ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ParticleManager;
	}
	return instance;
}

void ParticleManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ParticleManager::Initialize(DirectXCommon* birectxcommon, SrvManager* srvmanager, Camera* camera) {
	// NULL検出
	assert(birectxcommon);
    assert(camera);
	// メンバ変数に記録
	this->dxCommon_ = birectxcommon;
	this->srvmanager_ = srvmanager;
    this->camera_ = camera;
	// 乱数エンジンを初期化
	std::random_device rd;// 乱数生成器
	randomEngine = std::mt19937(rd());
    //ビルボード行列作成
    backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
}

void ParticleManager::SetParticleModel(Model* model, const std::string& directorypath, const std::string& filename) {
    assert(model);
    // メンバ変数に記録
    this->model_ = model;
    modelDate = LoadObjFile(directorypath, filename);
    // 頂点データを作成
    VertexDatacreation();
    // マテリアルの生成と初期化
    MaterialGenerate();
    // .objの参照しているテクスチャ読み込み
    TextureManager::GetInstance()->LoadTexture(modelDate.material.textureFilePath);
    // 読み込んだテクスチャの番号を取得
    modelDate.material.textureindex = TextureManager::GetInstance()->GetSrvIndex(modelDate.material.textureFilePath);
}

void ParticleManager::Update() {
    //ビルボード行列
    Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, camera_->GetWorludMatrix());
    billboardMatrix.m[3][0] = 0.0f;
    billboardMatrix.m[3][1] = 0.0f;
    billboardMatrix.m[3][2] = 0.0f;
    // ビュープロジェクション行列をカメラから取得
    Matrix4x4 viewMatrix = camera_->GetViewMatrix();
    Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();

    for (auto& [name, group] : particleGroups) {
        uint32_t counter = 0;
        for (std::list<Particle>::iterator particleIterator = group.particles.begin(); particleIterator != group.particles.end();) {  

            // world行列を計算
            Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
            Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);
            Matrix4x4 worldMatrix = Multiply(Multiply(scaleMatrix, billboardMatrix), translateMatrix);
            // waorldViewProjection行列
            Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

            if (counter < group.kNumInstance) {
                group.instanceData[counter].WVP = worldViewProjectionMatrix;
                group.instanceData[counter].World = worldMatrix;
                group.instanceData[counter].color = particleIterator->color;
                ++counter;
            }
            // 次のパーティクルに進む
            ++particleIterator;
        }
    }
}

void ParticleManager::Draw() {
    // RootSignature と PipelineState を設定
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(GraphicsPipeline::GetInstance()->GetRootSignatureParticle().Get());
    dxCommon_->GetCommandList()->SetPipelineState(GraphicsPipeline::GetInstance()->GetGraphicsPipelineStateParticle().Get());

    // プリミティブトポロジー（ここでは三角形リスト）を設定
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // パーティクルグループごとに描画処理を行う
    for (const auto& [name, particleGroup] : particleGroups) {
        // インスタンス数が0の場合は描画しない
        if (particleGroup.kNumInstance == 0) {
            continue;
        }
        // VertexBufferView を設定
        dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
        // マテリアル用の定数バッファを設定
        dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
        dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, TextureManager::GetInstance()->GetSrvHandleGPU(particleGroup.materialData.textureFilePath));
        
        std::cout << "Binding SRV for index: " << particleGroup.srvindex << std::endl;

        // インスタンシングデータの SRV を設定（テクスチャファイルのパスを指定）
        srvmanager_->SetGraphicsRootDescriptorTable(1, particleGroup.srvindex);
        // SRVで画像を表示
        srvmanager_->SetGraphicsRootDescriptorTable(2, particleGroup.materialData.textureindex);
        // 描画（インスタンシング）を実行
        dxCommon_->GetCommandList()->DrawInstanced(static_cast<UINT>(modelDate.vertices.size()), static_cast<UINT>(particleGroup.kNumInstance), 0, 0);
    }
}

void ParticleManager::VertexDatacreation() {
    // 関数化したResouceで作成
    vertexResoruce = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelDate.vertices.size());
    //頂点バッファビューを作成する
    // リソースの先頭のアドレスから使う
    vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    // 使用するリソースのサイズはの頂点のサイズ
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelDate.vertices.size());
    // 1頂点当たりのサイズ
    vertexBufferView.StrideInBytes = sizeof(VertexData);
    // 頂点リソースにデータを書き込むためのアドレスを取得
    vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    // 頂点データをリソースにコピー
    std::memcpy(vertexData, modelDate.vertices.data(), sizeof(VertexData) * modelDate.vertices.size());
}

void ParticleManager::MaterialGenerate() {
    // マテリアル用のリソース
    materialResource = dxCommon_->CreateBufferResource(sizeof(Model::Material));
    // マテリアル用にデータを書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // マテリアルデータの初期値を書き込む
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->endbleLighting = true;
    materialData->uvTransform = MakeIdentity4x4();
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilepath) {
    // マテリアルのテクスチャファイルパスでテクスチャをロード
    TextureManager::GetInstance()->LoadTexture(textureFilepath);
    // テクスチャインデックスを取得
    uint32_t textureIndex = TextureManager::GetInstance()->GetSrvIndex(textureFilepath);

    // 既にパーティクルグループが登録されていないことを確認
    assert(particleGroups.find(name) == particleGroups.end());

    // name で新しいパーティクルグループを取得
    ParticleGroup& newGroup = particleGroups[name];
    // 新しいパーティクルグループにテクスチャパスとインデックスを設定
    newGroup.materialData.textureFilePath = textureFilepath;
    newGroup.materialData.textureindex = textureIndex;

    // インスタンス用のリソースバッファを作成
    newGroup.Resource = dxCommon_->CreateBufferResource(sizeof(InstanceData) * MaxInstanceCount);
    newGroup.instanceData = nullptr;
    newGroup.Resource->Map(0, nullptr, reinterpret_cast<void**>(&newGroup.instanceData));

    // インスタンスデータを初期化
    for (uint32_t index = 0; index < MaxInstanceCount; ++index) {
        newGroup.instanceData[index].WVP = MakeIdentity4x4();
        newGroup.instanceData[index].World = MakeIdentity4x4();
        newGroup.instanceData[index].color = { 1.0f, 1.0f, 1.0f, 0.0f }; // 透明
    }
    // 書き込み後にリソースをアンマップ
    newGroup.Resource->Unmap(0, nullptr);
    // インスタンスバッファ用のSRVを割り当て、インデックスを記録
    newGroup.srvindex = srvmanager_->Allocate();
    // 構造体バッファ用のSRVを作成
    srvmanager_->CreateSRVforStructuredBuffer(newGroup.srvindex, newGroup.Resource.Get(), MaxInstanceCount, sizeof(InstanceData));
    // 新しいパーティクルグループをコンテナに登録
    particleGroups.emplace(name, std::move(newGroup));
}

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count) {
    // パーティクルグループが存在しない場合、エラーを発生させる
    auto it = particleGroups.find(name);
    if (it == particleGroups.end()) {
        assert(false);
        return;
    }
    // 既存のパーティクルグループを取得
    ParticleGroup& group = it->second;
    // 乱数範囲設定（-0.5 ~ 0.5 のランダムなオフセット）
    std::uniform_real_distribution<float> dist(-1.5f, 1.5f);

    // count 回のパーティクルを発生させる
    for (uint32_t i = 0; i < count; ++i) {
        Vector3 offset(dist(randomEngine), dist(randomEngine), dist(randomEngine)); // ランダムオフセット
        // パーティクルの位置と色を指定
        Vector4 color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);  // デフォルトの色（白）
        // 新しいパーティクルを生成
        Particle newParticle;
        newParticle.transform.translate.x = position.x + offset.x;  // パーティクルの位置を設定
        newParticle.transform.translate.y = position.y + offset.y;  // パーティクルの位置を設定
        newParticle.transform.translate.z = position.z;
        newParticle.transform.rotate = { 0.0f,0.0f,0.0f };
        newParticle.transform.scale = { 1.0f,1.0f,1.0f };
        newParticle.color = color;  // パーティクルの色を設定
        // パーティクルをグループに追加
        group.particles.push_back(newParticle);
    }
    // インスタンシングデータの更新（必要に応じて）
    group.kNumInstance += count; // インスタンス数を更新
}


ParticleManager::MaterialDate ParticleManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    ParticleManager::MaterialDate materialDate; // 構築するMaterialDate
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

ParticleManager::ModelDate ParticleManager::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    ParticleManager::ModelDate modelDate; // 構築するModelDate
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
            ParticleManager::VertexData triangle[3];
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
