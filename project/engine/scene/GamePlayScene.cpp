#include "GamePlayScene.h"
#include<TextureManager.h>
#include<ModelManager.h>
#include<SpriteCommon.h>
#include<Object3dCommon.h>
#include<Input.h>
#ifdef USE_IMGUI
#include<ImGuiManager.h>
#endif // USE_IMGUI
#include<SceneManager.h>
#include <ParticleCommon.h>
#include <ParticleManager.h>

void GamePlayScene::Finalize() {

}

void GamePlayScene::Initialize() {

    // カメラの初期化
    camera = std::make_unique<Camera>();
    camera->SetRotate(Vector3(0.0f, 0.0f, 0.0f));
    camera->SetTranslate(Vector3(0.0f, 0.0f, -1000.0f));
    Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());

    // カメラの現在の位置と回転を取得
    Cameraposition = camera->GetTranslate();
    Camerarotation = camera->GetRotate();

    // テクスチャを読み込む
    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");

    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");
    ModelManager::GetInstance()->LoadModel("monsterBallUV.obj");
    ModelManager::GetInstance()->LoadModel("fence.obj");

    // 音声ファイルを追加
    soundData = SoundLoader::GetInstance()->SoundLoadWave("Resources/Alarm01.wav");

#pragma region 最初のシーンの初期化

    // スプライトの初期化
    sprite = Sprite::Create("Resources/uvChecker.png", Vector2{ 0.0f,0.0f }, 0.0f, Vector2{ 360.0f,360.0f });

    // オブジェクト作成
    object3d = Object3d::Create("axis.obj", Transform({{1.0f, 1.0f, 1.0f}, {0.0f, 4.71f, 0.0f}, {0.0f, 0.0f, 0.0f}}));
    // カメラをセット
    object3d->SetCamera(camera.get());

    // パーティクル 
    // モデル生成
    ParticleManager::GetInstance()->SetParticleModel("Resources", "plane.obj");
    // テクスチャ生成
    ParticleManager::GetInstance()->CreateParticleGroup("Particles", "Resources/uvChecker.png");
    // 発生
    ParticleManager::GetInstance()->Emit("Particles", Vector3{ 0.0f, -0.5f, 0.0f }, 5);
    ParticleManager::GetInstance()->SetCamera(camera.get());

#pragma endregion 最初のシーンの初期化
    // 音声プレイフラグ
    soundfige = 0;
}

void GamePlayScene::Update() {

    if (Input::GetInstance()->Pushkey(DIK_SPACE) && soundfige == 0) {
        soundfige = 1;
    }

    if (soundfige == 1) {
        // 音声再生
        SoundPlayer::GetInstance()->SoundPlayWave(soundData, false);
        soundfige = 2;
    }

#pragma region  ImGuiの更新処理開始
    // デモウィンドウの表示
    //ImGui::ShowDemoWindow();   

    // スプライト
    //sprite->DebugUpdata();

    // object3d
    object3d->DebugUpdata();

    // Camera
    camera->DebugUpdata();

#pragma endregion ImGuiの更新処理終了
    
    /*-------------------------------------------*/
    /*--------------Cameraの更新処理---------------*/
    /*------------------------------------------*/

    camera->Update();


    /*-------------------------------------------------------------------------------------------------------*/
    /*-----------------------------------3Dオブジェクトの更新処理の開始------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------*/

    // 更新処理
    object3d->Update();

    ParticleManager::GetInstance()->Update();

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
    ImGuiManager::GetInstance()->End();
}

void GamePlayScene::Draw() {
    /*------------------------------------------------------------------------------------------------------*/
    /*----------------------------------3Dオブジェクトの描画処理開始--------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------------*/
    // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    Object3dCommon::GetInstance()->Commondrawing();

    object3d->Draw();


    /*------------------------------------------------------------------------------------------------------*/
    /*----------------------------------3Dオブジェクトの描画処理終了--------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------------*/



    /*----------------------------------------------------------------------------------------------------*/
    /*------------------------------------Spriteの描画処理開始----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/
    // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    SpriteCommon::GetInstance()->Commondrawing();
    
    //sprite->Draw();

    /*----------------------------------------------------------------------------------------------------*/
    /*------------------------------------Spriteの描画処理終了----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/

    // パーティクルの描画準備。パーティクルの描画に共通のグラフィックスコマンドを積む 
    ParticleCommon::GetInstance()->Commondrawing();

    ParticleManager::GetInstance()->Draw();

}