#include "ParticleManager.h"
#include <MatrixVector.h>
#include <cassert>
#include <ModelManager.h>
#include <TextureManager.h>
#include <numbers>
#include<ImGuiManager.h>
#include <externals/DirectXTex/d3dx12.h>
#include <Object3dCommon.h>

using namespace MatrixVector;
using namespace Microsoft::WRL;

// 静的メンバ変数の定義
std::unique_ptr<ParticleManager> ParticleManager::instance = nullptr;

// シングルトンインスタンスの取得
ParticleManager* ParticleManager::GetInstance() {
    if (!instance) {
        instance = std::make_unique<ParticleManager>();
    }
    return instance.get();
}

// 終了
void ParticleManager::Finalize() {
    instance.reset();
}

void ParticleManager::Initialize(DirectXCommon* birectxcommon, SrvManager* srvmanager) {
    // NULL検出
    assert(birectxcommon);
    assert(srvmanager);
    // メンバ変数に記録
    this->dxCommon_ = birectxcommon;
    this->srvmanager_ = srvmanager;
    // 乱数エンジンを初期化
    std::random_device rd;// 乱数生成器
    randomEngine = std::mt19937(rd());


}

void ParticleManager::Update() {

}

void ParticleManager::Draw() {

}