#include "Object3dCommon.h"

void Object3dCommon::Initialize(DirectXCommon* dxCommon) {
    // NULL検出
    assert(dxCommon);
    // 引数を受け取ってメンバ変数に記録する
    dxCommon_ = dxCommon;
    // グラフィックスパイプラインの生成
    GraphicsPipelineGenerate();
}

void Object3dCommon::RootSignatureGenerate() {






}

void Object3dCommon::GraphicsPipelineGenerate() {
    RootSignatureGenerate();



}