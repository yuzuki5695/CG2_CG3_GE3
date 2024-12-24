#include "Object3d.h"
#include "Object3dCommon.h"
#include<fstream>
#include<sstream>
#include <cassert>
#include "MatrixVector.h"
#include "ModelManager.h"

void Object3d::Initialize(Object3dCommon* object3dCommon) {
	// NULL検出
	assert(object3dCommon);
	// 引数で受け取ってメンバ変数に記録する
	this->object3dCommon = object3dCommon;
    // WVP,World用のリソースの生成、初期化
    TransformationMatrixGenerate();
    // 平行光源の生成,初期化
    DirectionalLightGenerate();

    // transform変数を作る
    transform = { {1.0f,1.0f,1.0f},{0.0f,3.0f,0.0f},{0.0f,-0.5f,0.0f} };  
    cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-700.0f} };
}

void Object3d::Update() {
    Matrix4x4 worludMatrix = MatrixVector::MakeAftineMatrix(transform.scale, transform.rotate, transform.translate);
    Matrix4x4 cameraMatrix = MatrixVector::MakeAftineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
    Matrix4x4 viewMatrix = MatrixVector::Inverse(cameraMatrix);
    Matrix4x4 projectionMatrix = MatrixVector::MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
    Matrix4x4 worldViewProjectionMatrix = MatrixVector::Multiply(worludMatrix, MatrixVector::Multiply(viewMatrix, projectionMatrix));
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

void Object3d::SetModel(const std::string& filePath) {
    // モデルを検索してセットする
    model = ModelManager::GetInstance()->FindModel(filePath);
}