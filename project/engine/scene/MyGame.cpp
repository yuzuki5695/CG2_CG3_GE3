#include "MyGame.h"

void MyGame::Finalize() {
    // 基底クラスの終了処理
    Framework::Finalize();
}

void MyGame::Initialize() {
    // 基底クラスの初期化処理
    Framework::Initialize();
}

void MyGame::Update() {
    // 基底クラスの更新処理
    Framework::Update();
}

void MyGame::Draw() {
    // 描画処理
    Framework::Draw();
}