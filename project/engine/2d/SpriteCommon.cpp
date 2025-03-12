#include "SpriteCommon.h"
#include"GraphicsPipeline.h"

using namespace Microsoft::WRL;

SpriteCommon* SpriteCommon::instance = nullptr;

SpriteCommon* SpriteCommon::GetInstance() {
    if (instance == nullptr) {
        instance = new SpriteCommon;
    }
    return instance;
}

void SpriteCommon::Finalize() {
    // GraphicsPipeline のリソースを解放
    GraphicsPipeline::GetInstance()->Finalize();
    delete instance;
    instance = nullptr;
}

void SpriteCommon::Initialize(DirectXCommon* dxCommon) {
    // 引数を受け取ってメンバ変数に記録する
    dxCommon_ = dxCommon;
    graphicsPipeline = GraphicsPipeline::GetInstance();
    graphicsPipeline->Initialize(dxCommon_);
    graphicsPipeline->GenerateSprite();
}

void SpriteCommon::Commondrawing() {
    // RootSignatureを設定。PSOに設定しているけど別途設定が必要
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline->GetRootSignatureSprite().Get());
    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline->GetGraphicsPipelineStateSprite().Get());
    // 形状を設定。PSOに設定しているものとはまた別。同じものを設定する
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}