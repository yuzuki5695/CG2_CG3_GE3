#include<string>
#include<format>
#include<dxgi1_6.h>
#include<dxgidebug.h>
#include<dxcapi.h>
#include<cmath>
#include<assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <MatrixVector.h>
#include<fstream>
#include<sstream>
#include"ResourceObject.h"
#include "Input.h"
#include "DirectXCommon.h"
#include"D3DResourceLeakChecker.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelCommon.h"
#include "Model.h"
#include"ModelManager.h"
#include "Camera.h"
#include"SrvManager.h"
#include "ParticleManager.h"

using namespace MatrixVector;

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    OutputDebugStringA("Hello,Directx!\n");

#pragma region ポインタ
    Input* input = nullptr;
    WinApp* winApp = nullptr;
    DirectXCommon* dxCommon = nullptr;
    SpriteCommon* spriteCommon = nullptr;
    Object3dCommon* object3dCommon = nullptr;
    ModelCommon* modelCommon = nullptr;
    SrvManager* srvManager = nullptr;
#pragma endregion ポインタ

    // ウィンドウ作成

    // WindowsAPIの初期化
    winApp = new WinApp();
    winApp->Initialize();

    // DirectXの初期化
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

    // SRVマネージャーの初期化
    srvManager = new SrvManager();
    srvManager->Initialize(dxCommon);

    // テクスチャマネージャーの初期化
    TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
    // 3Dモデルマネージャの初期化
    ModelManager::GetInstance()->Initialize(dxCommon);

    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
    std::string TexturePath01 = "Resources/uvChecker.png";
    std::string TexturePath02 = "Resources/monsterBall.png";

    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadTexture("plane.obj");
    ModelManager::GetInstance()->LoadTexture("axis.obj");
    std::string ModelPath01 = "plane.obj";
    std::string ModelPath02 = "axis.obj";


    // 汎用機能の初期化 

    // 入力の初期化
    input = new Input();
    input->Initialize(winApp);

#pragma region 基盤システムの初期化

    // スプライト共通部の初期化
    spriteCommon = new SpriteCommon;
    spriteCommon->Initialize(dxCommon);

    // 3Dオブジェクト共通部の初期化
    object3dCommon = new Object3dCommon;
    object3dCommon->Initialize(dxCommon);

    // カメラの初期化
    Camera* camera = new Camera();
    camera->SetRotate({ 0.0f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,0.0f,-700.0f });
    object3dCommon->SetDefaultCamera(camera);

#pragma endregion 基盤システムの初期化

#pragma region 最初のシーンの初期化

    // スプライトの初期化
    Sprite* sprite = new Sprite;
    sprite->Initialize(spriteCommon);
    sprite->Crrate(TexturePath01, { 0.0f,0.0f }, 0.0f, { 360.0f,360.0f });

    // 3Dモデルの初期化
    Model* model = new Model;;
    model->Initialize(ModelManager::GetInstance()->GetModelCommon(), "Resources", ModelPath01);

    // 3Dオブジェクトの初期化
    Object3d* object3d = new Object3d;
    object3d->Initialize(object3dCommon);
    // オブジェクト作成
    object3d->Crrate(ModelPath01, { { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.0f, 0.0f }, { 0.0f, -0.5f, 0.0f } });

    // パーティクル初期化
    ParticleManager::GetInstance()->Initialize(dxCommon, srvManager, camera, model, "Resources", ModelPath01);
    // パーティクルグループの生成
    ParticleManager::GetInstance()->CreateParticleGroup("particl", TexturePath01);
    ParticleManager::GetInstance()->Emit("particl", Vector3{ 0.0f, -0.5f, 0.0f }, 4);

#pragma endregion 最初のシーンの初期化

    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;

    std::vector<Sprite*> sprites;
    const uint32_t spritesize = 6;
    float Position[spritesize]{};
    for (uint32_t i = 0; i < spritesize; ++i) {
        Position[i] = 180.0f * i;
        Sprite* sprite = new Sprite();
        sprite->Initialize(spriteCommon);
        sprite->Crrate(TexturePath01,{ 0.0f,0.0f }, 0.0f, { 90.0f ,90.0f });
        if (i % 2 == 1) {
            sprite->SetTexture(TexturePath02);
        }
        // 現在の位置を取得
        Vector2 position = sprite->GetPosition();
        // 位置を変更する
        position.x = Position[i];
        position.y = 100.0f;
        // 変更した座標を設定
        sprite->SetPosition(position);
        // 情報を転送
        sprites.push_back(sprite);
    }

    std::vector<Object3d*> objects;
    const uint32_t objectize = 2;
    float objectsPosition[spritesize]{};
    objectsPosition[0] = 2.0f;
    objectsPosition[1] = -2.0f;
    for (uint32_t i = 0; i < objectize; ++i) {
        Object3d* object3d = new Object3d();
        object3d->Initialize(object3dCommon);
        if (i % 2 == 0) {
            object3d->Crrate(ModelPath02, { { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.0f, 0.0f }, { 0.0f, -0.5f, 0.0f } });
        } else {
            object3d->Crrate(ModelPath01, { { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.0f, 0.0f }, { 0.0f, -0.5f, 0.0f } });
        }
        // 現在の位置を取得
        Vector3 position = object3d->GetTranslate();
        position.x = objectsPosition[i];
        // 変更した座標を設定
        object3d->SetTranslate(position);
        // 情報を転送
        objects.push_back(object3d);
    }

    // カメラの現在の位置と回転を取得
    Vector3 Cameraposition = camera->GetTranslate();
    Vector3 Camerarotation = camera->GetRotate();

    // ウィンドウの×ボタンが押されるまでループ
    while (true) {
        // Windowのメッセージ処理
        if (winApp->ProcessMessage()) {
            // ゲームループを抜ける
            break;
        }
        // ゲームの処理

        // 入力の更新
        input->Update();

        // 0を押している間true
        if (input->Pushkey(DIK_0)) {
            OutputDebugStringA("Hit 0 \n");
        }

        //ImGui_ImplDX12_NewFrame();
        //ImGui_ImplWin32_NewFrame();
        //ImGui::NewFrame();

        //// 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
        //ImGui::ShowDemoWindow();

        //ImGui::Begin("Sprite");
       /* ImGui::DragFloat3("scale", &transform.scale.x, 0.01f);
        ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f);
        ImGui::DragFloat3("translate", &transform.translate.x, 0.01f);
        ImGui::ColorEdit3("colorSprite", reinterpret_cast<float*>(materialSpriteDate));
        ImGui::Checkbox("useMonsterBall", &useMonsterBall);
        ImGui::DragFloat3("LightDirection", &directionalLightDate->direction.x, 0.01f);
        ImGui::DragFloat("LightIntensity", &directionalLightDate->intensity, 0.01f);
        ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);*/
        //  ImGui::End();

        //ImGui::Begin("Camera");
        //// カメラの位置を編集
        //ImGui::Text("Camera Transform");
        //ImGui::DragFloat3("Position", &Cameraposition.x, 0.1f);
        //ImGui::DragFloat("rotateX", &Camerarotation.x, 0.0001f, -0.01f, 0.01f, "%.6f");
        //ImGui::DragFloat("rotateY", &Camerarotation.y, 0.0001f, -0.01f, 0.01f, "%.6f");
        //ImGui::DragFloat("rotateZ", &Camerarotation.z, 0.0001f, -0.01f, 0.01f, "%.6f");;
        //ImGui::End();


        ////ImGuiの描画コマンドを生成
        //ImGui::Render();

        /*-------------------------------------------*/
        /*--------------カメラの更新処理---------------*/
        /*------------------------------------------*/
        camera->Update();

        camera->SetTranslate(Cameraposition);
        camera->SetRotate(Camerarotation);

        /*-------------------------------------------------------------------------------------------------------*/
        /*-----------------------------------3Dオブジェクトの更新処理の開始------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/


        // 更新処理
       // object3d->Update();

        //size_t index = 0;
        //size_t maxIterations = 2;
        //for (Object3d* object3d : objects) {
        //    if (index >= maxIterations) {
        //        break; // 指定回数を超えたらループを終了
        //    }
        //    object3d->Update();
        //    Vector3 rotation = object3d->GetRotate();
        //    if (index == 0) {
        //        rotation.z += 0.01f;
        //    } else if (index == 1) {
        //        rotation.y += 0.01f;
        //    }
        //    object3d->SetRotate(rotation);
        //    // インクリメントして次へ
        //    ++index;
        //}


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
        
        //  描画用のDescriptorHeapの設定
        srvManager->PreDraw();

        //  DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
        dxCommon->PreDraw();
        
        // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
        object3dCommon->Commondrawing();

        // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
        spriteCommon->Commondrawing();

        /*------------------------------------------------------------------------------------------------------*/
        /*----------------------------------3Dオブジェクトの描画処理開始--------------------------------------------*/
        /*-----------------------------------------------------------------------------------------------------*/

#pragma region 全てのObject3d個々の描画
      
       // object3d->Draw();

        for (Object3d* object3d : objects) {
           // object3d->Draw();
        }

        ParticleManager::GetInstance()->Draw();

#pragma endregion 全てのObject3d個々の描画

        /*------------------------------------------------------------------------------------------------------*/
        /*----------------------------------3Dオブジェクトの描画処理終了--------------------------------------------*/
        /*-----------------------------------------------------------------------------------------------------*/


        /*----------------------------------------------------------------------------------------------------*/
        /*------------------------------------Spriteの描画処理開始----------------------------------------------*/
        /*---------------------------------------------------------------------------------------------------*/

#pragma region 全てのSprite個々の描画

        //// Spriteの描画は常にuvCheckerにする
        //dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

       // sprite->Draw();
     
#pragma endregion 全てのSprite個々の描画

        /*----------------------------------------------------------------------------------------------------*/
        /*------------------------------------Spriteの描画処理終了----------------------------------------------*/
        /*---------------------------------------------------------------------------------------------------*/

       ////実際のcommandListのImGuiの描画コマンドを積む
       //ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList().Get());

        // 描画後処理
        dxCommon->PostDrow();
    }

    // シーンの解放
    delete  spriteCommon;
    delete  object3dCommon;
    delete modelCommon;

    // 汎用機能の解放
    delete  sprite;

    //sprites.clear();

    for (Sprite* sprite : sprites) {
        delete sprite;
    }
    
    // 3Dモデルの解放
    delete model;
    
    // 3Dオブジェクトの解放
    delete  object3d;
    for (Object3d* object3d : objects) {
        delete object3d;
    }
    // パーティクルマネージャの開放
    ParticleManager::GetInstance()->Finalize();
    // 入力解放
    delete input;
    // テクスチャマネージャーの終了
    TextureManager::GetInstance()->Finalize();
    // 3Dモデルマネージャの終了
    ModelManager::GetInstance()->Finalize();
    // SRVマネージャの開放
    delete srvManager;
    // DirectXの解放
    delete dxCommon;
    // ウィンドウ解放 
    // WindowsAPIの終了処理
    winApp->Finalize();
    // WindowsAPIの解放
    delete winApp;
    return 0;
}