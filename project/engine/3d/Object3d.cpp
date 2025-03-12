#include "Object3d.h"
#include "Object3dCommon.h"
#include<fstream>
#include<sstream>
#include <cassert>
#include "MatrixVector.h"
#include "ModelManager.h"
#include<ImGuiManager.h>

using namespace MatrixVector;

void Object3d::Initialize(Object3dCommon* object3dCommon) {
    // NULL検出
    assert(object3dCommon);
    // 引数で受け取ってメンバ変数に記録する
    this->object3dCommon = object3dCommon;
    // WVP,World用のリソースの生成、初期化
    TransformationMatrixGenerate();
    // 平行光源の生成,初期化
    DirectionalLightGenerate();
}

void Object3d::Update() {
    Matrix4x4 worludMatrix = MakeAftineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 worldViewProjectionMatrix;
    if (camera) {
        const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
        worldViewProjectionMatrix = Multiply(worludMatrix, viewProjectionMatrix);
    } else {
        worldViewProjectionMatrix = worludMatrix;
    }
    transformationMatrixData->World = worludMatrix;
    transformationMatrixData->WVP = worldViewProjectionMatrix;
}

void Object3d::Draw() {
    // 座標変化行列CBufferの場所を設定
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
    // 平行光源用のCBufferの場所を設定 
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

    // 3Dモデルが割り当てられていれば描画する
    if (model) {
        model->Draw();
    }
}

void Object3d::TransformationMatrixGenerate() {
    // WVP,World用のリソースを作る。TransformationMatrixを用意する
    transformationMatrixResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::TransformationMatrix));
    // データを書き込むためのアドレスを取得
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
    // 単位行列を書き込んでおく
    transformationMatrixData->WVP = MakeIdentity4x4();
    transformationMatrixData->World = MakeIdentity4x4();
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

void Object3d::SetModel(const std::string& filePath) {
    // モデルを検索してセットする
    model = ModelManager::GetInstance()->FindModel(filePath);
}

std::shared_ptr<Object3d> Object3d::Create(std::string& filePath, Transform transform) {
    auto object3d = std::make_shared<Object3d>();
    if (object3d == nullptr) {
        return nullptr;
    }
    // 初期化
    object3d->Initialize(Object3dCommon::GetInstance());
    // モデルを検索してセットする
    object3d->SetModel(filePath);
    object3d->transform_ = transform;
    object3d->camera = Object3dCommon::GetInstance()->GetDefaultCamera();
    return object3d;
}

void Object3d::DebugUpdata() {
#ifdef USE_IMGUI
    // ウィンドウサイズを指定
    ImGui::SetNextWindowSize(ImVec2(500, 100));
    ImGui::Begin("Object3d");
    ImGui::SliderFloat3("position", &transform_.translate.x, 0.0f, 0.1f, "%.01f");
    ImGui::End();
#endif // USE_IMGUI
}