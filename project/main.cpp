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
#include <cstdlib>  // std::rand(), std::srand()
#include <ctime>    // std::time()

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
    TextureManager::GetInstance()->LoadTexture("Resources/title.png");
    TextureManager::GetInstance()->LoadTexture("Resources/clear.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Rule01.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number10.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number09.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number08.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number07.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number06.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number05.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number04.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number03.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number02.png");
    TextureManager::GetInstance()->LoadTexture("Resources/Number01.png");
    TextureManager::GetInstance()->LoadTexture("Resources/operation.png");
    TextureManager::GetInstance()->LoadTexture("Resources/SPACE.png");
    TextureManager::GetInstance()->LoadTexture("Resources/RSPACE.png");

    std::string TexturePath01 = "Resources/uvChecker.png";
    std::string TexturePath02 = "Resources/monsterBall.png";
    std::string TexturePath03 = "Resources/title.png";
    std::string TexturePath04 = "Resources/clear.png";
    std::string TexturePath05 = "Resources/Rule01.png";
    std::string TexturePath06 = "Resources/operation.png";
    std::string TexturePath07 = "Resources/SPACE.png";
    std::string TexturePath08 = "Resources/RSPACE.png";
    // テクスチャパスを配列で管理
    std::string TexturePaths[] = {
        "Resources/Number10.png",
        "Resources/Number09.png",
        "Resources/Number08.png",
        "Resources/Number07.png",
        "Resources/Number06.png",
        "Resources/Number05.png",
        "Resources/Number04.png",
        "Resources/Number03.png",
        "Resources/Number02.png",
        "Resources/Number01.png"
    };


    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadTexture("plane.obj");
    ModelManager::GetInstance()->LoadTexture("axis.obj");
    ModelManager::GetInstance()->LoadTexture("cube.obj");
    ModelManager::GetInstance()->LoadTexture("skydome.obj");
    ModelManager::GetInstance()->LoadTexture("Player.obj");
    ModelManager::GetInstance()->LoadTexture("Enemy01.obj");
    ModelManager::GetInstance()->LoadTexture("Enemy02.obj");
    ModelManager::GetInstance()->LoadTexture("Enemy03.obj");
    std::string ModelPath01 = "plane.obj";
    std::string ModelPath02 = "axis.obj";
    std::string ModelPath03 = "cube.obj";
    std::string ModelPath04 = "skydome.obj";
    std::string ModelPath05 = "Player.obj";
    std::string ModelPath06 = "Enemy01.obj";
    std::string ModelPath07 = "Enemy02.obj";
    std::string ModelPath08 = "Enemy03.obj";

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
    Sprite* Titlesprite = new Sprite;
    Titlesprite->Initialize(spriteCommon,TexturePath03);
    Titlesprite->SetTextureSize(Vector2{ 446.0f,98.0f });
    Titlesprite->SetSize(Vector2{ 446.0f,98.0f });
    Titlesprite->SetPosition(Vector2{405.0f,150.0f});

    // スプライトの初期化
    Sprite* Clearsprite = new Sprite;
    Clearsprite->Initialize(spriteCommon, TexturePath04);
    Clearsprite->SetTextureSize(Vector2{ 259.0f,97.0f });
    Clearsprite->SetSize(Vector2{ 259.0f,97.0f });
    Clearsprite->SetPosition(Vector2{ 475.0f,150.0f });

    Sprite* Rule01 = new Sprite;
    Rule01->Initialize(spriteCommon, TexturePath05);
    Rule01->SetTextureSize(Vector2{ 270.0f,40.0f });
    Rule01->SetSize(Vector2{ 270.0f,40.0f });
    Rule01->SetPosition(Vector2{ 10.0f,10.0f });

    Sprite* TextureNumber = new Sprite;
    TextureNumber->Initialize(spriteCommon, TexturePaths[0]);
    TextureNumber->SetTextureSize(Vector2{ 80.0f,40.0f });
    TextureNumber->SetSize(Vector2{ 80.0f,40.0f });
    TextureNumber->SetPosition(Vector2{ 10.0f,60.0f });

    Sprite* operation = new Sprite;
    operation->Initialize(spriteCommon, TexturePath06);
    operation->SetTextureSize(Vector2{ 110.0f,80.0f });
    operation->SetSize(Vector2{ 110.0f,80.0f });
    operation->SetPosition(Vector2{ 10.0f,620.0f });

    Sprite* SPACE = new Sprite;
    SPACE->Initialize(spriteCommon, TexturePath07);
    SPACE->SetTextureSize(Vector2{ 280.0f,40.0f });
    SPACE->SetSize(Vector2{ 280.0f,40.0f });
    SPACE->SetPosition(Vector2{ 470.0f,270.0f });
    
    Sprite* RSPACE = new Sprite;
    RSPACE->Initialize(spriteCommon, TexturePath08);
    RSPACE->SetTextureSize(Vector2{ 240.0f,40.0f });
    RSPACE->SetSize(Vector2{ 240.0f,40.0f });
    RSPACE->SetPosition(Vector2{ 480.0f,270.0f });

    // プレイヤーの初期化
    Player* player = new Player;
    player->Initialize(object3dCommon, ModelPath05, input);

    // 敵の初期化
    const uint32_t enemysize = 10;
    std::vector<Enemy*> enemys;
    float enemysPosition[enemysize] = {100.0f,100.0f ,100.0f ,100.0f ,100.0f ,100.0f ,100.0f,100.0f,100.0f,100.0f };
    uint32_t enemyindex[enemysize] = { 0,1,0,2,1,2,0,1,2,0 };
    // モデルのパスを定義
    std::string ModelPaths[] = {
        "Enemy01.obj",  // ModelPath06
        "Enemy02.obj",  // ModelPath07
        "Enemy03.obj"   // ModelPath08
    };

    for (uint32_t i = 0; i < enemysize; ++i) {
        // ランダムなモデルパスを選択
        int randomIndex = std::rand() % 3;
        std::string selectedModel = ModelPaths[randomIndex];

        Enemy* enemy = new Enemy;
        enemy->Initialize(object3dCommon, ModelPaths[randomIndex]);
        // 現在の位置を取得
        Vector3 position = enemy->GetTranslate();
        position.x = enemysPosition[i];
        // 移動パターンを設定（enemyindex を保存）
        enemy->SetMovePattern(enemyindex[i]);
        // 変更した座標を設定
        enemy->SetTranslate(position);
        // ベクターに追加
        enemys.push_back(enemy);
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

    int name = 0;
    bool enemyfige = false;

    enum Direction {
        Title = 0,
        Game = 1,
        Clear = 2
    };

    // ウィンドウの×ボタンが押されるまでループ
    while (true) {
        // Windowのメッセージ処理
        if (winApp->ProcessMessage()) {
            // ゲームループを抜ける
            break;
        }
        // ゲームの処理

        if (input->Pushkey(DIK_R)) {
            name = 0;
        }

        // 入力の更新
        input->Update();
       
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

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

        size_t enemyscount = 0;

        switch (name) {
        case Title: 
            if (enemyfige) {
                enemycount = 0;
                enemyfige = false;
            }
            player->SetTranslate(Vector3{0.0f,0.0f,0.0f});;

            for (Enemy* enemy : enemys) {
                enemy->SetTranslate(Vector3(enemysPosition[enemycount]));
                enemy->SetisDead(false);
                enemyscount++;
            }

            enemyscount = 0;
            // シーン移動
            if (input->Pushkey(DIK_RETURN)) {
                name = 1;
            }
            break;
        case Game:
            // シーン移動
            if (enemycount == enemysize) {
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
        case Clear:
            // シーン移動
            if (input->Pushkey(DIK_R)) {
                name = 0;
                enemyfige = true;
                enemycount = 0;
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

            Titlesprite->Update();
            SPACE->Update();
            break;
        case Game:

            Rule01->Update();
            TextureNumber->Update();
            operation->Update();

            break;
        case Clear:
            Clearsprite->Update(); 
            RSPACE->Update();
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
        case Clear:


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

            Titlesprite->Draw();
            SPACE->Draw();
            break;
        case Game:
           
            Rule01->Draw();
            // enemycount の値で画像を切り替え
            if (enemycount >= 0 && enemycount <= 9) {
                TextureNumber->SetTexture(TexturePaths[enemycount]);
                TextureNumber->Draw();
            } 
            operation->Draw();

            break;
        case Clear:

            if (enemycount == enemysize) {
                Clearsprite->Draw();
            }
            RSPACE->Draw();
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
    delete  Titlesprite;
    delete Clearsprite;
    delete Rule01;
    delete TextureNumber;
    delete  operation;
    delete SPACE;
    delete RSPACE;

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