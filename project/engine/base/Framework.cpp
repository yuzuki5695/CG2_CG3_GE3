#include "Framework.h"

void Framework::Run() {
    // ゲームの初期化
    Initialize();

    while (true) // ゲームループ
    {
        // 毎フレーム更新
        Update();
        // 終了リクエストが来たらループを抜ける
        if (IsEndRequst()) {
            break;
        }
        // 描画
        Draw();
    }
    // ゲームの終了
    Finalize();
}

void Framework::Finalize() {
    // シーンの解放
    delete spriteCommon;
    delete object3dCommon;
    delete modelCommon;
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
    // 音声データ解放
    soundLoader->Finalize();
    delete soundLoader;
    delete soundPlayer;
    // DirectXの解放
    delete dxCommon;
    // WindowsAPIの解放
    delete winApp;
    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;
}

void Framework::Initialize() {
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

#pragma region 基盤システムの初期化

    // 入力の初期化
    input = new Input();
    input->Initialize(winApp);

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

    // カメラの現在の位置と回転を取得
    Cameraposition = camera->GetTranslate();
    Camerarotation = camera->GetRotate();

#pragma endregion 基盤システムの初期化
}

void Framework::Update() {
    // Windowのメッセージ処理
    if (winApp->ProcessMessage()) {
        // ゲームループを抜ける
        endRequst_ = true;
    }
    // 入力の更新
    input->Update();
    // ImGuiの受付開始
    imGuiManager->Begin();
    /*-------------------------------------------*/
    /*--------------カメラの更新処理---------------*/
    /*------------------------------------------*/
    camera->Update();
    camera->SetTranslate(Cameraposition);
    camera->SetRotate(Camerarotation);
}