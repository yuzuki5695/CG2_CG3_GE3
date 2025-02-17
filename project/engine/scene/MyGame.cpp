#include "MyGame.h"

void MyGame::Finalize() {
    // 汎用機能の解放
    delete  sprite;
    // 3Dモデルの解放
    delete model;
    // 3Dオブジェクトの解放
    delete  object3d;
    // 音声データ解放
    Framework::GetSoundPlayer()->SoundUnload(&soundData);
    // 基底クラスの終了処理
    Framework::Finalize();
}

void MyGame::Initialize() {
    // 基底クラスの初期化処理
    Framework::Initialize();
    // テクスチャを読み込む
    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
    // 変数に代入
    TexturePath01 = "Resources/uvChecker.png";
    TexturePath02 = "Resources/monsterBall.png";

    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");
    // 変数に代入
    ModelPath01 = "plane.obj";
    ModelPath02 = "axis.obj";

    // 音声ファイル
    soundData = Framework::GetSoundLoader()->SoundLoadWave("Resources/Alarm01.wav");

#pragma region 最初のシーンの初期化

    // スプライトの初期化
    sprite = new Sprite;
    sprite->Initialize(Framework::GetSpriteCommon());
    sprite->Create(TexturePath01, { 100.0f,100.0f }, 0.0f, { 360.0f,360.0f });

    // 3Dモデルの初期化
    model = new Model;;
    model->Initialize(ModelManager::GetInstance()->GetModelCommon(), "Resources", ModelPath01);

    // 3Dオブジェクトの初期化
    object3d = new Object3d;
    object3d->Initialize(Framework::GetObject3dCommon());
    // オブジェクト作成
    object3d->Create(ModelPath01, { { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.0f, 0.0f }, { 0.0f, -0.5f, 0.0f } });

#pragma endregion 最初のシーンの初期化
    // 音声プレイフラグ
    soundfige = 0;
}

void MyGame::Update() {
    // 基底クラスの更新処理
    Framework::Update();

    // 0を押している間true
    if (Framework::GetInput()->Pushkey(DIK_0)) {
        OutputDebugStringA("Hit 0 \n");
    }


    if (Framework::GetInput()->Pushkey(DIK_SPACE) && soundfige == 0) {
        soundfige = 1;
    }

    if (soundfige == 1) {
        // 音声再生
        Framework::GetSoundPlayer()->SoundPlayWave(soundData, false);
        soundfige = 2;
    }

#pragma region  ImGuiの更新処理開始
    // デモウィンドウの表示
    //ImGui::ShowDemoWindow();
    // スプライト
    sprite->DebugUpdata();
#pragma endregion ImGuiの更新処理終了

    /*-------------------------------------------------------------------------------------------------------*/
    /*-----------------------------------3Dオブジェクトの更新処理の開始------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------*/

    // 更新処理
    object3d->Update();

    /*-------------------------------------------------------------------------------------------------------*/
    /*-----------------------------------3Dオブジェクトの更新処理の終了------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------*/


    /*----------------------------------------------------------------------------------------------------*/
    /*---------------------------------------Spriteの更新処理----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/

    // 更新処理
    sprite->Update();

    /*----------------------------------------------------------------------------------------------------*/
    /*-------------------------------------Spriteの更新処理終了----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/
    // ImGuiの描画前準備
    Framework::GetImGuiManager()->End();
}

void MyGame::Draw() {
    //  描画用のDescriptorHeapの設定
    Framework::GetSrvManager()->PreDraw();
    //  DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
    Framework::GetDirectXCommon()->PreDraw();
    // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    Framework::GetObject3dCommon()->Commondrawing();
    // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    Framework::GetSpriteCommon()->Commondrawing();

    /*------------------------------------------------------------------------------------------------------*/
    /*----------------------------------3Dオブジェクトの描画処理開始--------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------------*/

    object3d->Draw();

    /*------------------------------------------------------------------------------------------------------*/
    /*----------------------------------3Dオブジェクトの描画処理終了--------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------------*/


    /*----------------------------------------------------------------------------------------------------*/
    /*------------------------------------Spriteの描画処理開始----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/

    //// Spriteの描画は常にuvCheckerにする
    //dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

    sprite->Draw();

    /*----------------------------------------------------------------------------------------------------*/
    /*------------------------------------Spriteの描画処理終了----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/
    // ImGuiの描画開始
    Framework::GetImGuiManager()->Draw();
    // 描画後処理
    Framework::GetDirectXCommon()->PostDrow();
}