#include "MyGame.h"

void MyGame::Finalize() {
    // シーンの終了処理
    scene_->Finalize();
    // シーンの解放
    delete scene_;
    // 基底クラスの終了処理
    Framework::Finalize();
}

void MyGame::Initialize() {
    // 基底クラスの初期化処理
    Framework::Initialize();
    // ゲームプレイシーンの生成
    scene_ = new GamePlayScene();
    // ゲームプレイシーンの初期化
    scene_->Initialize();
}

void MyGame::Update() {
    // 基底クラスの更新処理
    Framework::Update();
    // シーンの更新処理
    scene_->Update();
}

void MyGame::Draw() {
    //  描画用のDescriptorHeapの設定
    Framework::GetSrvManager()->PreDraw();
    //  DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
    Framework::GetDirectXCommon()->PreDraw();
    // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    Object3dCommon::GetInstance()->Commondrawing();
    // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    SpriteCommon::GetInstance()->Commondrawing();

    // シーンの描画処理
    scene_->Draw();

    // ImGuiの描画開始
    ImGuiManager::GetInstance()->Draw();
    // 描画後処理
    Framework::GetDirectXCommon()->PostDrow();
}