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
#include "Camera.h"
#include "Player.h"
#include "Enemy.h"

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
    ModelManager::GetInstance()->LoadTexture("cube.obj");
    ModelManager::GetInstance()->LoadTexture("skydome.obj");
    std::string ModelPath01 = "plane.obj";
    std::string ModelPath02 = "axis.obj";
    std::string ModelPath03 = "cube.obj";
    std::string ModelPath04 = "skydome.obj";
    
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
    sprite->Initialize(spriteCommon, "Resources/uvChecker.png");

    // 3Dモデルの初期化
    Model* model = new Model;;
    model->Initialize(ModelManager::GetInstance()->GetModelCommon(), "Resources", ModelPath01);

    // 3Dオブジェクトの初期化
    Object3d* object3d = new Object3d;
    object3d->Initialize(object3dCommon);
    // モデルを結びつける
    object3d->SetModel(ModelPath01);

    // プレイヤーの初期化
    Player* player = new Player;
    player->Initialize(object3dCommon, ModelPath03, input);

    // 敵の初期化
    Enemy* enemy = new Enemy;
    enemy->Initialize(object3dCommon, ModelPath03);


#pragma endregion 最初のシーンの終了

    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;

    // カメラの現在の位置と回転を取得
    Vector3 Cameraposition = camera->GetTranslate();
    Vector3 Camerarotation = camera->GetRotate();

    std::vector<Object3d*> enemys;
    const uint32_t enemysize = 3;
    float enemysPosition[enemysize]{};
    enemysPosition[0] = 5.0f;
    enemysPosition[1] = 10.0f;
    enemysPosition[2] = 15.0f;
    for (uint32_t i = 0; i < enemys; ++i) {
        Object3d* object3d = new Object3d();
        object3d->Initialize(object3dCommon);
        // 現在の位置を取得
        Vector3 position = object3d->GetTranslate();
        position.x = enemysPosition[i];
        // 変更した座標を設定
        object3d->SetTranslate(position);
        // 情報を転送
        enemys.push_back(object3d);
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
       // ImGui::ShowDemoWindow();

        //ImGui::Begin("Camera");
        //// カメラの位置を編集
        //ImGui::Text("Camera Transform");
        //ImGui::DragFloat3("Position", &Cameraposition.x, 0.1f);
        //ImGui::DragFloat("rotateX", &Camerarotation.x, 0.0001f, -0.01f, 0.01f, "%.6f");
        //ImGui::DragFloat("rotateY", &Camerarotation.y, 0.0001f, -0.01f, 0.01f, "%.6f");
        //ImGui::DragFloat("rotateZ", &Camerarotation.z, 0.0001f, -0.01f, 0.01f, "%.6f");;
        //ImGui::End();

          //ImGuiの描画コマンドを生成
        ImGui::Render();

        /*-------------------------------------------*/
        /*--------------カメラの更新処理---------------*/
        /*------------------------------------------*/
        camera->Update();

        camera->SetTranslate(Cameraposition);
        camera->SetRotate(Camerarotation);

        /*-------------------------------------------------------------------------------------------------------*/
        /*-----------------------------------3Dオブジェクトの更新処理の開始------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/


        // プレイヤーの更新処理
        player->Update();

        // 敵の更新
        enemy->Update();


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


        /*-------------------------------------------------------------------------------------------------------*/
        /*-----------------------------------3Dオブジェクトの更新処理の終了------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/


        /*-------------------------------------------------------------------------------------------------------*/
        /*--------------------------------------Spriteの更新処理の開始----------------------------------------------*/
        /*-------------------------------------------------------------------------------------------------------*/


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

        // 敵の描画処理
        enemy->Draw();

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



#pragma endregion 全てのSprite個々の描画
        /*----------------------------------------------------------------------------------------------------*/
        /*------------------------------------Spriteの描画処理終了----------------------------------------------*/
        /*---------------------------------------------------------------------------------------------------*/

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

    delete enemy;

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