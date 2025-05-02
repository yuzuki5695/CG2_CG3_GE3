#include "Object3d.h"
#include "Object3dCommon.h"
#include<fstream>
#include<sstream>
#include <cassert>
#include "MatrixVector.h"
#include "ModelManager.h"
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI

using namespace MatrixVector;

void Object3d::Initialize(Object3dCommon* object3dCommon) {
    // NULL検出
    assert(object3dCommon);
    // 引数で受け取ってメンバ変数に記録する
    this->object3dCommon = object3dCommon;
    this->camera = object3dCommon->GetDefaultCamera();
    // WVP,World用のリソースの生成、初期化
    TransformationMatrixGenerate();
    // 平行光源の生成,初期化
    DirectionalLightGenerate();
    // カメラリソースの生成、初期化
    CameraForGPUGenerate();
}

void Object3d::Update() {
    // ワールド行列の作成
    Matrix4x4 worldMatrix = MakeAftineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    // ワールド・ビュー・プロジェクション行列
    Matrix4x4 worldViewProjectionMatrix;
    if (camera) {
        const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
        worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
    } else {
        worldViewProjectionMatrix = worldMatrix;
    }
    transformationMatrixData->WVP = worldViewProjectionMatrix;
    transformationMatrixData->World = worldMatrix;
    // WorldInverseTranspose行列を再計算
    transformationMatrixData->WorldInverseTranspose = InverseTranspose(worldMatrix);
}

void Object3d::Draw() {
    // 座標変化行列CBufferの場所を設定
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
    // 平行光源用のCBufferの場所を設定 
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
    // カメラの場所を設定 
    object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());

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
    // WorldInverseTransposeを計算してセット
    transformationMatrixData->WorldInverseTranspose = InverseTranspose(transformationMatrixData->World);
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

void Object3d::CameraForGPUGenerate(){
    // カメラ用リソースを作る
    cameraResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Object3d::CameraForGPU));
    // 書き込むためのアドレスを取得
    cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));
    // 単位行列を書き込んでおく
    cameraForGPUData->worldPosition = { 0.0f, 0.0f, -500.0f };
}

void Object3d::SetModel(const std::string& filePath) {
    // モデルを検索してセットする
    model = ModelManager::GetInstance()->FindModel(filePath);
}

std::unique_ptr<Object3d> Object3d::Create(std::string filePath, Transform transform) {
    std::unique_ptr<Object3d> object3d = std::make_unique<Object3d>();   
    // 初期化
    object3d->Initialize(Object3dCommon::GetInstance());
    // モデルを検索してセットする
    object3d->model = ModelManager::GetInstance()->FindModel(filePath);
    // 座標をセット
    object3d->transform_ = transform;
  
    return object3d;
}

void Object3d::DebugUpdata(const std::string& name) {
#ifdef USE_IMGUI
    // 引数で受け取った name を使ってウィンドウを作成
    ImGui::Begin(name.c_str());  // 渡された名前を使ってウィンドウを表示
    // オブジェクトの座標
    ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
    ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
    ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
    // カラー
    ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&model->GetMaterialData()->color));
    // ライトの向き
    ImGui::DragFloat3("LightDirection", &directionalLightDate->direction.x, 0.01f);
    // ライトの輝度
    ImGui::DragFloat("LightIntensity", &directionalLightDate->intensity, 0.01f);
    // 光沢度
    ImGui::DragFloat("Shinimess", &model->GetMaterialData()->shinimess, 0.01f);
    ImGui::End();
#endif // USE_IMGUI

}