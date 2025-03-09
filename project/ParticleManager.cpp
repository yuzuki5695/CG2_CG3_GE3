#include "ParticleManager.h"
#include <MatrixVector.h>
#include <cassert>
#include <ModelManager.h>
#include <TextureManager.h>
#include <numbers>
#include <GraphicsPipeline.h>
#include<ImGuiManager.h>
#include <externals/DirectXTex/d3dx12.h>

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

void ParticleManager::Initialize(DirectXCommon* birectxcommon, SrvManager* srvmanager) {
    // NULL検出
    assert(birectxcommon);
    // メンバ変数に記録
    this->dxCommon_ = birectxcommon;
    this->srvmanager_ = srvmanager;
    // 乱数エンジンを初期化
    std::random_device rd;// 乱数生成器
    randomEngine = std::mt19937(rd());
    // グラフィックスパイプラインの生成
    GraphicsPipelineGenerate();
}

void ParticleManager::SetParticleModel(Camera* camera, Model* model, const std::string& directorypath, const std::string& filename) {
    // NULL検出
    assert(camera);
    // メンバ変数に記録
    this->camera_ = camera;
    this->model_ = model;
    // モデルデータを取得
    modelDate = LoadObjFile(directorypath, filename);
    // 頂点データを作成
    VertexDatacreation();
    // マテリアルの生成と初期化
    MaterialGenerate();
    // .objの参照しているテクスチャ読み込み
    TextureManager::GetInstance()->LoadTexture(modelDate.material.textureFilePath);
    // 読み込んだテクスチャの番号を取得
    modelDate.material.textureindex = TextureManager::GetInstance()->GetSrvIndex(modelDate.material.textureFilePath);
    //ビルボード行列作成
    backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
}

void ParticleManager::Update() {
    // ビルボード行列
    Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, camera_->GetWorludMatrix());
    billboardMatrix.m[3][0] = 0.0f;
    billboardMatrix.m[3][1] = 0.0f;
    billboardMatrix.m[3][2] = 0.0f;

    // ビュープロジェクション行列をカメラから取得
    Matrix4x4 viewMatrix = camera_->GetViewMatrix();
    Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();

    // 各パーティクルグループの処理
    for (auto& [name, group] : particleGroups) {
        uint32_t counter = 0;
        for (auto particleIterator = group.particles.begin(); particleIterator != group.particles.end();) {
            // パーティクルの現在の時間を増加させる
            (*particleIterator).currentTime += 1.0f / 60.0f;  // 60fpsで時間をカウントアップ

            // パーティクルの寿命が尽きたら削除
            if ((*particleIterator).currentTime >= (*particleIterator).lifetime) {
                particleIterator = group.particles.erase(particleIterator);  // パーティクル削除
                continue;
            }

            // パーティクルの位置や動きを更新
            (*particleIterator).transform.translate.x += (*particleIterator).Velocity.x * (1.0f / 60.0f);

            // 透明度の更新（時間に基づいてフェード）
            float alpha = 1.0f - (*particleIterator).currentTime / (*particleIterator).lifetime;
            (*particleIterator).color.w = alpha;

            // world行列の計算
            Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
            Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);
            Matrix4x4 worldMatrix = Multiply(Multiply(scaleMatrix, billboardMatrix), translateMatrix);

            // worldViewProjection行列の計算
            Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

            // インスタンスデータに更新した行列を設定
            if (counter < group.kNumInstance) {
                group.instanceData[counter].WVP = worldViewProjectionMatrix;
                group.instanceData[counter].World = worldMatrix;
                group.instanceData[counter].color = (*particleIterator).color;
                ++counter;
            }

            // 次のパーティクルに進む
            ++particleIterator;
        }

        // パーティクル発生の処理（一定時間ごとに新しいパーティクルを生成）
        if (group.spawnTime >= group.spawnFrequency) {
            // 新しいパーティクルを発生させる
            Emit("Particles", Vector3{ 0.0f, 0.0f, 0.0f }, 1);  // 実際の発生処理を呼び出す
            group.spawnTime = 0.0f;  // 発生時間をリセット
        } else {
            // 発生時間を増加
            group.spawnTime += 1.0f / 60.0f;
        }
    }
}

void ParticleManager::Draw() {
    // RootSignature と PipelineState を設定
    //dxCommon_->GetCommandList()->SetGraphicsRootSignature(GraphicsPipeline::GetInstance()->GetRootSignatureParticle().Get());
    //dxCommon_->GetCommandList()->SetPipelineState(GraphicsPipeline::GetInstance()->GetGraphicsPipelineStateParticle().Get());

    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());


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
    // すでにテクスチャがロードされているか確認
    if (!TextureManager::GetInstance()->IsTextureLoaded(textureFilepath)) {
        // マテリアルのテクスチャファイルをロード
        TextureManager::GetInstance()->LoadTexture(textureFilepath);
    }

    // パーティクルグループ名が既に存在するかチェック
    auto it = particleGroups.find(name);
    if (it != particleGroups.end()) {
        // 名前が一致するグループが存在する場合、そのグループのテクスチャが一致するか確認
        if (it->second.materialData.textureFilePath == textureFilepath) {
            // テクスチャが一致する場合、既存のグループを再利用
            return;
        } else {
            // テクスチャが異なる場合、既存のグループを更新
            it->second.materialData.textureFilePath = textureFilepath;
            it->second.materialData.textureindex = TextureManager::GetInstance()->GetSrvIndex(textureFilepath);
            // 必要に応じてリソースの再割り当てなどを行うことができます
        }
    } else {
        // 新しいパーティクルグループを作成
        ParticleGroup& newGroup = particleGroups[name];

        // 新しいパーティクルグループにテクスチャパスとインデックスを設定
        newGroup.materialData.textureFilePath = textureFilepath;
        newGroup.materialData.textureindex = TextureManager::GetInstance()->GetSrvIndex(textureFilepath);

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
    }
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

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count) {
    // パーティクルグループが存在しない場合、エラーを発生させる
    auto it = particleGroups.find(name);
    if (it == particleGroups.end()) {
        assert(false);
        return;
    }

    // 既存のパーティクルグループを取得
    ParticleGroup& group = it->second;

    // 現在のパーティクル数がMaxInstanceCountを超えている場合、追加するパーティクル数を調整
    uint32_t currentParticleCount = group.particles.size();
    if (currentParticleCount + count > MaxInstanceCount) {
        count = MaxInstanceCount - currentParticleCount;  // 最大数を超えないように調整
    }

    std::uniform_real_distribution<float> dist(-1.5f, 1.5f);

    // count 回のパーティクルを発生させる
    for (uint32_t i = 0; i < count; ++i) {
        // ランダムオフセットを適用
        Vector3 offset(dist(randomEngine), dist(randomEngine), dist(randomEngine));

        // パーティクルの位置と色を指定
        Vector4 color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);  // 初期色は白

        // 新しいパーティクルを生成
        Particle newParticle;
        newParticle.transform.translate = { position.x + offset.x, position.y + offset.y, position.z + offset.z };
        newParticle.transform.rotate = { 0.0f, 0.0f, 0.0f };  // 回転の初期化（必要に応じて後で変更可能）
        newParticle.transform.scale = { 1.0f, 1.0f, 1.0f };  // スケールの初期化
        newParticle.color = color;
        newParticle.lifetime = 1.0f;  // 新しく発生したパーティクルのライフタイムを設定
        newParticle.currentTime = 0.0f;  // 寿命の初期化
        newParticle.Velocity = { 1.0f,1.0f,1.0f };
        group. spawnTime = 0.0f;
        group.spawnFrequency = 0.5f;
        // パーティクルをグループに追加
        group.particles.push_back(newParticle);
    }

    // インスタンシングデータの更新
    group.kNumInstance += count;  // 新たに追加したインスタンス数を反映
}

void ParticleManager::DebugUpdata() {
    // テクスチャ選択のコンボボックス
    ImGui::Text("Select texture for particles:");
    const char* textureOptions[] = { "uvChecker", "circle" }; // 使用するテクスチャの選択肢
    static int textureIndex = 0;  // 選択中のインデックス

    // テクスチャ選択UI
    if (ImGui::Combo("Texture", &textureIndex, textureOptions, IM_ARRAYSIZE(textureOptions))) {
        // textureIndexが変更されるたびにテクスチャを即座に適用
        const char* textureFilePath = nullptr;
        switch (textureIndex) {
        case 0: textureFilePath = "Resources/uvChecker.png"; break;
        case 1: textureFilePath = "Resources/circle.png"; break;
        default: textureFilePath = "Resources/uvChecker.png"; break; // デフォルト設定
        }

        // 即座にテクスチャをロードして適用
        TextureManager::GetInstance()->LoadTexture(textureFilePath); // テクスチャをロード
        const char* particleGroupName = (textureIndex == 0) ? "uvChecker" : "circle"; // パーティクルグループ名を固定

        // テクスチャを適用してパーティクルグループを更新
        CreateParticleGroup(particleGroupName, textureFilePath);
    }

    // テクスチャパスのマッピング
    const char* textureFilePath = (textureIndex == 0) ? "Resources/uvChecker.png" : "Resources/circle.png";
    ImGui::Text("Selected Texture: %s", textureFilePath);  // 現在選択されているテクスチャを表示

    // パーティクル発生ボタン
    if (ImGui::Button("Emit Particles")) {
        // パーティクルを発生させる
        Emit((textureIndex == 0) ? "uvChecker" : "circle", { 0.0f, 0.0f, 0.0f }, 1);
    }
}

void ParticleManager::RootSignatureGenerate() {

    HRESULT hr;

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------DescriptorRange作成-------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // Instancing作成
    D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
    descriptorRangeForInstancing[0].BaseShaderRegister = 0; // 0から始める
    descriptorRangeForInstancing[0].NumDescriptors = 1; // 数は1つ
    descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
    descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------RootParameter作成---------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_ROOT_PARAMETER rootParameters[4] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[0].Descriptor.ShaderRegister = 0;// レジスタ番号0を使う

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う
    rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing; // Tableの中身の配列を指定
    rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing); // Tableで利用する数

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//利用する数

    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[3].Descriptor.ShaderRegister = 1;// レジスタ番号1を使う

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------RootSignature作成---------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.pParameters = rootParameters;// ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = _countof(rootParameters);// 配列の長さ

    /*----------------------------------------------------------------------------------*/
    /*-----------------------------------Samplerの設定-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
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

    /*----------------------------------------------------------------------------------*/
    /*-------------------------シリアライズしてバイナリにする--------------------------------*/
    /*----------------------------------------------------------------------------------*/
    ComPtr <ID3DBlob> signatureBlob = nullptr;
    ComPtr <ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    // バイナリを元に作成
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));
}

void ParticleManager::GraphicsPipelineGenerate() {
    // ルートシグネチャの生成
    RootSignatureGenerate();
    HRESULT hr;

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------InputLayout設定-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
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

    /*----------------------------------------------------------------------------------*/
    /*---------------------------------BlendStateの設定-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
    D3D12_BLEND_DESC blendDesc{};
    //全ての色要素を書き込む
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;           // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;         // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;         // すでに書き込まれている色 (デストカラー)

    //// 加算合成
    //blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;       // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    //blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;           // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    //blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;            // すでに書き込まれている色 (デストカラー)

    //// 減算合成
    //blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;         // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    //blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;    // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    //blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;              // すでに書き込まれている色 (デストカラー)

    //// 乗算合成
    //blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;           // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    //blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;         // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    //blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;    // すでに書き込まれている色 (デストカラー)

    //// スクリーン合成
    //blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;    // これから書き込む色。PixeShaderから出力する色 (ソースカラ―)
    //blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;             // これから書き込むα。PixeShaderから出力するα値 (ソースアルファ)
    //blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;              // すでに書き込まれている色 (デストカラー)


    //===== RasterizerStateの設定を行う ======//   
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    // 裏面(時計回り)を表示しない
    //rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    // 裏面(時計回り)を表紙する
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    // 三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    /*----------------------------------------------------------------------------------*/
    /*--------------------------------ShaderをCompile-----------------------------------*/
    /*----------------------------------------------------------------------------------*/
    ComPtr <IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    ComPtr <IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Particle.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    /*-----------------------------------------------------------------------------------*/
    /*-------------------------------------PSO生成----------------------------------------*/
    /*-----------------------------------------------------------------------------------*/
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
    // 利用するトポロジ(形状)のタイプ
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // どのように画面に色を打ち込むかの設定(気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = dxCommon_->GetdepthStencilDesc();
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 実際に生成
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
    assert(SUCCEEDED(hr));
}