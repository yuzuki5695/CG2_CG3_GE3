#include<string>
#include<format>
#include<dxgi1_6.h>
#include<dxgidebug.h>
#include<dxcapi.h>
#include<cmath>
#include<assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include"MatrixVector.h"
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
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"

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
#pragma endregion ポインタ

    // ウィンドウ作成
    
    // WindowsAPIの初期化
    winApp = new WinApp();
    winApp->Initialize();

    // DirectXの初期化
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

    // テクスチャマネージャーの初期化
    TextureManager::GetInstance()->Initialize(dxCommon);

    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
    std::string TexturePath01 = "Resources/uvChecker.png";
    std::string TexturePath02 = "Resources/monsterBall.png";

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

#pragma endregion 基盤システムの初期化

#pragma region 最初のシーンの初期化

    Sprite* sprite = new Sprite;
    sprite->Initialize(spriteCommon);
    sprite->Crrate(TexturePath01, { 0.0f,0.0f }, 0.0f, { 360.0f,360.0f });

    // 3Dオブジェクトの初期化
    Object3d* object3d = new Object3d;
    object3d->Initialize(object3dCommon);

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

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
        ImGui::ShowDemoWindow();

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


        //ImGuiの描画コマンドを生成
        ImGui::Render();

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

        // 描画用のDescriptorHeapの設定
        ID3D12DescriptorHeap* descriptorHeap[] = { dxCommon->GetsrvDescriptorHeap().Get()};
        dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeap);

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
      
        object3d->Draw();

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

        sprite->Draw();
     
#pragma endregion 全てのSprite個々の描画

        /*----------------------------------------------------------------------------------------------------*/
        /*------------------------------------Spriteの描画処理終了----------------------------------------------*/
        /*---------------------------------------------------------------------------------------------------*/

       //実際のcommandListのImGuiの描画コマンドを積む
       ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList().Get());

        // 描画後処理
        dxCommon->PostDrow();
    }

    // シーンの解放
    delete  spriteCommon;
    delete  object3dCommon;

    // 汎用機能の解放
    delete  sprite;

    //sprites.clear();

    for (Sprite* sprite : sprites) {
        delete sprite;
    }

    // 3Dオブジェクトの解放
    delete  object3d;

    // 入力解放
    delete input;

    // テクスチャマネージャーの終了
    TextureManager::GetInstance()->Finalize();

    // DirectXの解放
    delete dxCommon;

    // ウィンドウ解放 
    // WindowsAPIの終了処理
    winApp->Finalize();
    // WindowsAPIの解放
    delete winApp;
    return 0;
}