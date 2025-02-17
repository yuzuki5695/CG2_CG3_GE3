#include "MyGame.h"

void MyGame::Finalize() {
    // 基底クラスの終了処理
    Framework::Finalize();
}

void MyGame::Initialize() {
    // 基底クラスの初期化処理
    Framework::Initialize();
    // ゲームプレイシーンの生成
    BaseScene* scene_ = new TitleScene();
    // シーンマネージャに最初のシーンをセット
    SceneManager::GetInstance()->SetNextScene(scene_);
}

void MyGame::Update() {
    // 基底クラスの更新処理
    Framework::Update();
}

void MyGame::Draw() {
    // 描画処理
    Framework::Draw();
    // シーンマネージャの描画処理
    SceneManager::GetInstance()->Draw();
    // デバックテキスト描画
    ImGuiManager::GetInstance()->Draw();
    // 描画後処理
    Framework::GetDirectXCommon()->PostDrow();
}