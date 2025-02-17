#include "MyGame.h"

void MyGame::Finalize() {
    // シーンの解放
    delete  spriteCommon;
    delete  object3dCommon;
    delete modelCommon;
    // 汎用機能の解放
    delete  sprite;
    // 3Dモデルの解放
    delete model;
    // 3Dオブジェクトの解放
    delete  object3d;
    // 入力解放
    delete input;
    // カメラ
    delete camera;
    // テクスチャマネージャーの終了
    TextureManager::GetInstance()->Finalize();
    // 3Dモデルマネージャの終了
    ModelManager::GetInstance()->Finalize();
    // ImGuiマネージャの解放
    delete imGuiManager;
    // SRVマネージャの開放
    delete srvManager;
    // 音声再生
    soundPlayer->SoundUnload(&soundData);
    // 音声データ解放
    delete soundLoader;
    delete soundPlayer;
    // DirectXの解放
    delete dxCommon;
    // WindowsAPIの解放
    delete winApp;
    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;
}

void MyGame::Initialize() {
    OutputDebugStringA("Hello,Directx!\n");

    // ウィンドウ作成
    // WindowsAPIの初期化
    winApp = new WinApp();
    winApp->Initialize();
    // DirectXの初期化
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);
    // 音声読み込み
    soundLoader = new SoundLoader();
    soundLoader->Initialize();
    // 音声再生
    soundPlayer = new SoundPlayer();
    soundPlayer->Initialize(soundLoader);

    // SRVマネージャーの初期化
    srvManager = new SrvManager();
    srvManager->Initialize(dxCommon);
    // ImGuiマネージャの初期化
    imGuiManager = new ImGuiManager();
    imGuiManager->Initialize(winApp, dxCommon, srvManager);
    // テクスチャマネージャーの初期化
    TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
    // 3Dモデルマネージャの初期化
    ModelManager::GetInstance()->Initialize(dxCommon);

    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
    TexturePath01 = "Resources/uvChecker.png";
    TexturePath02 = "Resources/monsterBall.png";

    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadModel("plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");
    ModelPath01 = "plane.obj";
    ModelPath02 = "axis.obj";

    // 音声ファイル
    soundData = soundLoader->SoundLoadWave("Resources/Alarm01.wav");

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
    camera = new Camera();
    camera->SetRotate({ 0.0f,0.0f,0.0f });
    camera->SetTranslate({ 0.0f,0.0f,-700.0f });
    object3dCommon->SetDefaultCamera(camera);

#pragma endregion 基盤システムの初期化

#pragma region 最初のシーンの初期化

    // スプライトの初期化
    sprite = new Sprite;
    sprite->Initialize(spriteCommon);
    sprite->Create(TexturePath01, { 100.0f,100.0f }, 0.0f, { 360.0f,360.0f });

    // 3Dモデルの初期化
    model = new Model;;
    model->Initialize(ModelManager::GetInstance()->GetModelCommon(), "Resources", ModelPath01);

    // 3Dオブジェクトの初期化
    object3d = new Object3d;
    object3d->Initialize(object3dCommon);
    // オブジェクト作成
    object3d->Create(ModelPath01, { { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.0f, 0.0f }, { 0.0f, -0.5f, 0.0f } });

#pragma endregion 最初のシーンの初期化

    // カメラの現在の位置と回転を取得
    Cameraposition = camera->GetTranslate();
    Camerarotation = camera->GetRotate();

    // 音声プレイフラグ
    soundfige = 0;
}

void MyGame::Update() {
    // Windowのメッセージ処理
    if (winApp->ProcessMessage()) {
        // ゲームループを抜ける
        endRequst_ = true;
    }
    // ゲームの処理

    // 入力の更新
    input->Update();

    // 0を押している間true
    if (input->Pushkey(DIK_0)) {
        OutputDebugStringA("Hit 0 \n");
    }


    if (input->Pushkey(DIK_SPACE) && soundfige == 0) {
        soundfige = 1;
    }

    if (soundfige == 1) {
        // 音声再生
        soundPlayer->SoundPlayWave(soundData, false);
        soundfige = 2;
    }

#pragma region  ImGuiの更新処理開始
    // ImGuiの受付開始
    imGuiManager->Begin();
    // デモウィンドウの表示
    //ImGui::ShowDemoWindow(); 

    // スプライト
    sprite->DebugUpdata();

#pragma endregion ImGuiの更新処理終了

    /*-------------------------------------------------------------------------------------------------------------------------------*/
    /*--------------------------------------------------------更新処理の開始-----------------------------------------------------------*/
    /*-------------------------------------------------------------------------------------------------------------------------------*/

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
    imGuiManager->End();

    /*-------------------------------------------------------------------------------------------------------------------------------*/
    /*--------------------------------------------------------更新処理の終了-----------------------------------------------------------*/
    /*-------------------------------------------------------------------------------------------------------------------------------*/
}

void MyGame::Draw() {
    //  描画用のDescriptorHeapの設定
    srvManager->PreDraw();
    //  DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
    dxCommon->PreDraw();
    // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    object3dCommon->Commondrawing();
    // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    spriteCommon->Commondrawing();

    /*-------------------------------------------------------------------------------------------------------------------------------*/
    /*--------------------------------------------------------描画処理の開始-----------------------------------------------------------*/
    /*-------------------------------------------------------------------------------------------------------------------------------*/

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

    /*-------------------------------------------------------------------------------------------------------------------------------*/
    /*--------------------------------------------------------描画処理の終了-----------------------------------------------------------*/
    /*-------------------------------------------------------------------------------------------------------------------------------*/
    // ImGuiの描画開始
    imGuiManager->Draw();
    // 描画後処理
    dxCommon->PostDrow();
}