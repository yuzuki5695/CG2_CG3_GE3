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

void GamePlayScene::Finalize() {
    // カメラ
    delete camera;
    // 汎用機能の解放
    delete  sprite;
    // 3Dオブジェクトの解放
    delete  object3d;
    // 音声データ解放
    SoundPlayer::GetInstance()->SoundUnload(&soundData);
}

void GamePlayScene::Initialize() {

    // カメラの初期化
    camera = new Camera();
    camera->SetRotate(Vector3(0.0f, 0.0f, 0.0f));
    camera->SetTranslate(Vector3(0.0f, 0.0f, -1000.0f));
    Object3dCommon::GetInstance()->SetDefaultCamera(camera);

    // カメラの現在の位置と回転を取得
    Cameraposition = camera->GetTranslate();
    Camerarotation = camera->GetRotate();


    // テクスチャを読み込む
    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
    // 変数に代入
    TexturePath01 = "Resources/uvChecker.png";
    TexturePath02 = "Resources/monsterBall.png";

    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");
    ModelManager::GetInstance()->LoadModel("monsterBallUV.obj");
    // 変数に代入
    ModelPath01 = "monsterBallUV.obj";
    ModelPath02 = "axis.obj";

    // 音声ファイルを追加
    soundData = SoundLoader::GetInstance()->SoundLoadWave("Resources/Alarm01.wav");

#pragma region 最初のシーンの初期化

    // スプライトの初期化
    sprite = new Sprite;
    sprite->Initialize(SpriteCommon::GetInstance());
    sprite->Create(TexturePath01, { 100.0f,100.0f }, 0.0f, { 360.0f,360.0f });

    // 3Dオブジェクトの初期化
    object3d = new Object3d;
    object3d->Initialize(Object3dCommon::GetInstance());
    // オブジェクト作成
    object3d->Create(ModelPath01, { { 1.0f, 1.0f, 1.0f }, { 0.0f, 4.71f, 0.0f }, { 0.0f, 0.0f, 0.0f } });
    object3d->SetCamera(camera);

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
    sprite->DebugUpdata();

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

    object3d->Draw();

    /*------------------------------------------------------------------------------------------------------*/
    /*----------------------------------3Dオブジェクトの描画処理終了--------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------------*/


    /*----------------------------------------------------------------------------------------------------*/
    /*------------------------------------Spriteの描画処理開始----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/

    sprite->Draw();

    /*----------------------------------------------------------------------------------------------------*/
    /*------------------------------------Spriteの描画処理終了----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/
}