#include "TitleScene.h"
#include<TextureManager.h>
#include<ModelManager.h>
#include<SpriteCommon.h>
#include<Object3dCommon.h>
#include<Input.h>
#include<ImGuiManager.h>
#include<SceneManager.h>
#include"ParticleManager.h"

void TitleScene::Finalize() {
    // 汎用機能の解放 
    // パーティクルマネージャの開放
    ParticleManager::GetInstance()->Finalize();
    delete  sprite;
    delete  model;
    // カメラ
    delete camera;
}

void TitleScene::Initialize() {
    // テクスチャを読み込む
    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
    TextureManager::GetInstance()->LoadTexture("Resources/circle.png");
    // 変数に代入
    TexturePath01 = "Resources/uvChecker.png";
    TexturePath02 = "Resources/monsterBall.png";
    TexturePath03 = "Resources/circle.png";

    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadModel("plane.obj"); 
    // 変数に代入
    ModelPath01 = "plane.obj";

    // 音声ファイルを追加
    soundData = SoundLoader::GetInstance()->SoundLoadWave("Resources/Alarm01.wav");

#pragma region 最初のシーンの初期化

    // スプライトの初期化
    sprite = new Sprite;
    sprite->Initialize(SpriteCommon::GetInstance());
    sprite->Create(TexturePath01, { 100.0f,100.0f }, 0.0f, { 360.0f,360.0f });

    // 3Dモデルの初期化
    model = new Model;
    model->Initialize(ModelManager::GetInstance()->GetModelCommon(), "Resources", ModelPath01);

    // カメラの初期化
    camera = new Camera();
    camera->SetRotate({ 0.0f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,0.0f,-700.0f });
    Object3dCommon::GetInstance()->SetDefaultCamera(camera);

    // カメラの現在の位置と回転を取得
    Cameraposition = camera->GetTranslate();
    Camerarotation = camera->GetRotate();

    ParticleManager::GetInstance()->SetParticleModel(camera, model, "Resources", ModelPath01);
    ParticleManager::GetInstance()->CreateParticleGroup("Particles", TexturePath01);
    ParticleManager::GetInstance()->CreateParticleGroup("uvChecker", TexturePath01);
    ParticleManager::GetInstance()->Emit("Particles", Vector3{ 0.0f, -0.5f, 0.0f }, 1);

#pragma endregion 最初のシーンの初期化
    // 音声プレイフラグ
    soundfige = 0;
}

void TitleScene::Update() {
    // ENTERキーを押したら
    if (Input::GetInstance()->Triggrkey(DIK_RETURN)) {
        // シーン切り替え
      //  SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

    // 0を押している間true
    if (Input::GetInstance()->Pushkey(DIK_0)) {
        OutputDebugStringA("Hit 0 \n");
    }

    if (Input::GetInstance()->Triggrkey(DIK_SPACE) && soundfige == 0) {
        //soundfige = 1;
    }

    if (soundfige == 1) {
        // 音声再生
        SoundPlayer::GetInstance()->SoundPlayWave(soundData, false);
        soundfige = 2;
    }

#pragma region  ImGuiの更新処理開始
    // スプライト
   // sprite->DebugUpdata();
   // パーティクル
    ParticleManager::GetInstance()->DebugUpdata();

   
#pragma endregion ImGuiの更新処理終了
    /*-------------------------------------------*/
    /*--------------カメラの更新処理---------------*/
    /*------------------------------------------*/
    camera->Update();
    camera->SetTranslate(Cameraposition);
    camera->SetRotate(Camerarotation);

    /*-------------------------------------------------------------------------------------------------------*/
    /*-----------------------------------3Dオブジェクトの更新処理の開始------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------*/

    // パーティクルの更新処理
    ParticleManager::GetInstance()->Update();

    /*-------------------------------------------------------------------------------------------------------*/
    /*-----------------------------------3Dオブジェクトの更新処理の終了------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------*/


    /*----------------------------------------------------------------------------------------------------*/
    /*---------------------------------------Spriteの更新処理----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/

    // 更新処理
   // sprite->Update();

    /*----------------------------------------------------------------------------------------------------*/
    /*-------------------------------------Spriteの更新処理終了----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/
    // ImGuiの描画前準備
    ImGuiManager::GetInstance()->End();
}

void TitleScene::Draw() {
    /*------------------------------------------------------------------------------------------------------*/
    /*----------------------------------3Dオブジェクトの描画処理開始--------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------------*/

    // パーティクルの描画処理 
    ParticleManager::GetInstance()->Draw();

    /*------------------------------------------------------------------------------------------------------*/
    /*----------------------------------3Dオブジェクトの描画処理終了--------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------------*/


    /*----------------------------------------------------------------------------------------------------*/
    /*------------------------------------Spriteの描画処理開始----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/

    //// Spriteの描画は常にuvCheckerにする
    //dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

    //sprite->Draw();

    /*----------------------------------------------------------------------------------------------------*/
    /*------------------------------------Spriteの描画処理終了----------------------------------------------*/
    /*---------------------------------------------------------------------------------------------------*/
}