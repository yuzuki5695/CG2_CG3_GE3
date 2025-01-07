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
#include "Skydome.h"

// AABB同士の衝突判定
static bool CheckCollisionAABB(const AABB& box1, const AABB& box2) {
    // X軸方向
    bool overlapX = (box1.left < box2.right) && (box1.right > box2.left);
    // Y軸方向
    bool overlapY = (box1.top < box2.bottom) && (box1.bottom > box2.top);

    // 両方の条件を満たして衝突
    return overlapX && overlapY;
}



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
    camera->SetTranslate({ 0.0f,0.0f,-5000.0f });
    object3dCommon->SetDefaultCamera(camera);

#pragma endregion 基盤システムの初期化

#pragma region 最初のシーンの初期化

    // スプライトの初期化
    Sprite* sprite = new Sprite;
    sprite->Initialize(spriteCommon, "Resources/uvChecker.png");

    // プレイヤーの初期化
    Player* player = new Player;
    player->Initialize(object3dCommon, ModelPath03, input);

    // 敵の初期化
    const uint32_t enemysize = 5;
    std::vector<Enemy*> enemys;
    float enemysPosition[enemysize] = { 5.0f ,10.0f ,15.0f,20.0f,25.0f };
\

    for (uint32_t i = 0; i < enemysize; ++i) {
\
        Enemy* enemy = new Enemy;
        enemy->Initialize(object3dCommon, ModelPath03);
        // 現在の位置を取得
        Vector3 position = enemy->GetTranslate();
        position.x = enemysPosition[i];
        // 変更した座標を設定
        enemy->SetTranslate(position);
        // ベクターに追加
        enemys.push_back(enemy);
\
    }

    // 天球の初期化
    Skydome* skydome = new Skydome;
    skydome->Initialize(object3dCommon, ModelPath04);

#pragma endregion 最初のシーンの終了
    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;

    // カメラの現在の位置と回転を取得
    Vector3 Cameraposition = camera->GetTranslate();
    Vector3 Camerarotation = camera->GetRotate();

    int enemycount = 0;

    Vector3 pos = skydome->GetScale();

    int name = 1;

    enum Direction {
        Title = 0,
        Game = 1,
        Criea = 2
    };

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
       
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
       // ImGui::ShowDemoWindow();

        ImGui::Begin("Camera");
        ImGui::InputInt("Enemy kill", &enemycount);
        ImGui::DragFloat3("skyPos", &pos.x, 0.1f);
        skydome->SetScale(pos);
        //// カメラの位置を編集
        //ImGui::Text("Camera Transform");
        //ImGui::DragFloat3("Position", &Cameraposition.x, 0.1f);
        //ImGui::DragFloat("rotateX", &Camerarotation.x, 0.0001f, -0.01f, 0.01f, "%.6f");
        //ImGui::DragFloat("rotateY", &Camerarotation.y, 0.0001f, -0.01f, 0.01f, "%.6f");
        //ImGui::DragFloat("rotateZ", &Camerarotation.z, 0.0001f, -0.01f, 0.01f, "%.6f");;
        ImGui::End();

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

        switch (name) {
        case Title:
            // シーン移動
            if (input->Pushkey(DIK_RETURN)) {
                name = 1;
            }

            player->SetTranslate(Vector3{0.0f,0.0f,0.0f});
            enemycount = 0;
            for (Enemy* enemy : enemys) {
                enemy->SetisDead(false);
            }

            break;
        case Game:
            // シーン移動
            if (enemycount == 5) {
                name = 2;
            }

            // プレイヤーの更新処理
            player->Update();

            AABB playerBox = player->GetAABB();  // プレイヤーのAABB

            // 敵の更新
            for (Enemy* enemy : enemys) {
                // 敵のAABBを取得
                AABB enemyBox = enemy->GetAABB();
                // プレイヤーと敵が衝突した場合の処理
                if (CheckCollisionAABB(playerBox, enemyBox) && !enemy->GetisDead()) {
                    enemy->OnCollision();  // 敵が衝突した際の処理
                    enemycount++;
                }
                enemy->Update();          
            }


            break;
        case Criea:
            // シーン移動
            if (input->Pushkey(DIK_R)) {
                name = 0;
            }

            break;
        }

        // 天球の更新
        skydome->Update();

        /*-------------------------------------------------------------------------------------------------------*/
        /*-----------------------------------3Dオブジェクトの更新処理の終了------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/


        /*-------------------------------------------------------------------------------------------------------*/
        /*--------------------------------------Spriteの更新処理の開始----------------------------------------------*/
        /*-------------------------------------------------------------------------------------------------------*/

        switch (name) {
        case Title:


            break;
        case Game:

         
            break;
        case Criea:

            sprite->Update();

            break;
        }

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

        // 天球の描画
        skydome->Draw();


        switch (name) {
        case Title:


            break;
        case Game:


            // プレイヤーの描画処理
            player->Draw();

            // 敵の描画処理
            for (Enemy* enemy : enemys) {
                enemy->Draw();
            }


            break;
        case Criea:


            break;
        }

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

        switch (name) {
        case Title:


            break;
        case Game:
           


            break;
        case Criea:

            if (enemycount == enemysize) {
                sprite->Draw();
            }

            break;
        }

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
    // delete model;
    // 3Dオブジェクトの解放
    // delete  object3d;

    delete player;

    for (Enemy* enemy : enemys) {
        delete enemy;
    }

    delete skydome;

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