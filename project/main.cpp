#include"MatrixVector.h"
#include"ResourceObject.h"
#include "Input.h"
#include "DirectXCommon.h"
#include"D3DResourceLeakChecker.h"
#include"Transform.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelCommon.h"
#include "Model.h"
#include"ModelManager.h"
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include "Player.h"

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

    // テクスチャマネージャの初期化
    TextureManager::GetInstance()->Initialize(dxCommon);
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

#pragma endregion 基盤システムの初期化

#pragma region 最初のシーンの初期化

    // スプライトの初期化
    Sprite* sprite = new Sprite;
    sprite->Initialize(spriteCommon, "Resources/uvChecker.png");

    // 3Dモデルの初期化
    Model* model = new Model;
    model->Initialize(ModelManager::GetInstance()->GetModelCommon(), "Resources", ModelPath01);

    // 3Dオブジェクトの初期化
    Object3d* object3d = new Object3d;
    object3d->Initialize(object3dCommon);
    // モデルを結びつける
    object3d->SetModel(ModelPath01);

    // プレイヤーの初期化
    Player* player = new Player;
    player->Initialize(object3dCommon, ModelPath02,input);

    // 敵の初期化



#pragma endregion 最初のシーンの終了


    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;

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
        if (input->Pushkey(DIK_D)) {
            OutputDebugStringA("Hit 0 \n");
        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        // 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
        //ImGui::ShowDemoWindow();
        
        ImGui::Begin("Sprite");
       /* ImGui::DragFloat3("scale", &transform.scale.x, 0.01f);
        ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f);
        ImGui::DragFloat3("translate", &transform.translate.x, 0.01f);
        ImGui::ColorEdit3("colorSprite", reinterpret_cast<float*>(materialSpriteDate));
        ImGui::Checkbox("useMonsterBall", &useMonsterBall);
        ImGui::DragFloat3("LightDirection", &directionalLightDate->direction.x, 0.01f);
        ImGui::DragFloat("LightIntensity", &directionalLightDate->intensity, 0.01f);
        ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);*/
        ImGui::End();

        /*-------------------------------------------------------------------------------------------------------*/
        /*-----------------------------------3Dオブジェクトの更新処理の開始------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/

        // プレイヤーの更新処理
        player->Update();


        /*-------------------------------------------------------------------------------------------------------*/
        /*-----------------------------------3Dオブジェクトの更新処理の終了------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/


        /*--------------------------------------------------------------------------------------------------------*/
        /*---------------------------------------priteの更新処理の開始----------------------------------------------*/
        /*-------------------------------------------------------------------------------------------------------*/

        // 更新処理
        sprite->Update();

        /*--------Spriteの回転--------*/
        // 角度を変化させるテスト
        float rotation = sprite->GetRotation();
        if (input->Pushkey(DIK_W)) {
            rotation -= 0.01f;
        }
        if (input->Pushkey(DIK_S)) {
            rotation += 0.01f;
        }
        sprite->SetRotation(rotation);

        /*-------------------------------------------------------------------------------------------------------*/
        /*-------------------------------------Spriteの更新処理の終了----------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/

        // 描画用のDescriptorHeapの設定
        ID3D12DescriptorHeap* descriptorHeap[] = { dxCommon->GetsrvDescriptorHeap().Get() };
        dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeap);

        //  DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
        dxCommon->PreDraw();

        /*------------------------------------------------------------------------------------------------------*/
        /*----------------------------------3Dオブジェクトの描画処理開始--------------------------------------------*/
        /*-----------------------------------------------------------------------------------------------------*/

        // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
        object3dCommon->Commondrawing();
#pragma region 全てのObject3d個々の描画

        // プレイヤーの描画処理
        player->Draw();



#pragma endregion 全てのObject3d個々の描画
        /*------------------------------------------------------------------------------------------------------*/
        /*----------------------------------3Dオブジェクトの描画処理終了--------------------------------------------*/
        /*-----------------------------------------------------------------------------------------------------*/

        /*----------------------------------------------------------------------------------------------------*/
        /*------------------------------------Spriteの描画処理開始----------------------------------------------*/
        /*---------------------------------------------------------------------------------------------------*/

        // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
        spriteCommon->Commondrawing();
#pragma region 全てのSprite個々の描画


        sprite->Draw();


#pragma endregion 全てのSprite個々の描画
        /*----------------------------------------------------------------------------------------------------*/
        /*------------------------------------Spriteの描画処理終了----------------------------------------------*/
        /*---------------------------------------------------------------------------------------------------*/

        //ImGuiの描画コマンドを生成
        ImGui::Render();

        //実際のcommandListのImGuiの描画コマンドを積む
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList().Get());

        // 描画後処理
        dxCommon->PostDrow();
    }

#pragma region 各処理の解放

    // ImGuiの終了処理。
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // シーンの解放
    delete  spriteCommon;
    delete  object3dCommon;
    // 汎用機能の解放

    // スプライトの解放
    delete  sprite;
    
    // 3Dモデルの解放
    delete model;
    
    // 3Dオブジェクトの解放
    
    delete  object3d;
    delete player;

    // 入力解放
    delete input;

    // テクスチャマネージャーの終了
    TextureManager::GetInstance()->Finalize();
    // 3Dモデルマネージャの終了
    ModelManager::GetInstance()->Finalize();
    // DirectXの解放
    delete dxCommon;

    // ウィンドウ解放 
    // WindowsAPIの終了処理
    winApp->Finalize();
    // WindowsAPIの解放
    delete winApp;
#pragma endregion 各処理の解放
    return 0;
}