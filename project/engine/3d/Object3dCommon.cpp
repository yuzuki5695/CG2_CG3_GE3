#include "Object3dCommon.h"
#include <cassert>
#include "GraphicsPipeline.h"

using namespace Microsoft::WRL;

Object3dCommon* Object3dCommon::instance = nullptr;

Object3dCommon* Object3dCommon::GetInstance() {
    if (instance == nullptr) {
        instance = new Object3dCommon;
    }
    return instance;
}

void Object3dCommon::Finalize() {
    delete instance;
    instance = nullptr;
}

void Object3dCommon::Initialize(DirectXCommon* dxCommon) {
    // NULL検出
    assert(dxCommon);
    // 引数を受け取ってメンバ変数に記録する
    dxCommon_ = dxCommon;
    graphicsPipeline_ = GraphicsPipeline::GetInstance();
    graphicsPipeline_->GenerateObject3d();
}

void Object3dCommon::Commondrawing() {

    // RootSignatureを設定。PSOに設定しているけど別途設定が必要
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureObject3d().Get());
    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateObject3d().Get());
    // 形状を設定。PSOに設定しているものとはまた別。同じものを設定する
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}