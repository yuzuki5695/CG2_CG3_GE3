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
    // シーンファクトリの解放
    delete sceneFactory_;
    // シーンマネージャの解放
    SceneManager::GetInstance()->Finalize();
    // 基盤システムの解放
    SpriteCommon::GetInstance()->Finalize();
    Object3dCommon::GetInstance()->Finalize();
    delete modelCommon;
    // 入力解放
    Input::GetInstance()->Finalize();
    // テクスチャマネージャーの終了
    TextureManager::GetInstance()->Finalize();
    // 3Dモデルマネージャの終了
    ModelManager::GetInstance()->Finalize();
    // ImGuiマネージャの解放
    ImGuiManager::GetInstance()->Finalize();
    // SRVマネージャの開放
    delete srvManager;
    // 音声データの解放
    //xAudio2解放
    SoundLoader::GetInstance()->GetIXAudio2();
    SoundLoader::GetInstance()->Finalize();
    SoundPlayer::GetInstance()->Finalize();
    // DirectXの解放
    delete dxCommon;
    // WindowsAPIの解放
    winApp.reset();
    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;
}

void Framework::Initialize() {
    OutputDebugStringA("Hello,Directx!\n");
    // ウィンドウ作成
    // WindowsAPIの初期化
    winApp = std::make_unique <WinApp>();
    winApp->Initialize();
    // DirectXの初期化
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp.get());
    // 音声読み込み
    SoundLoader::GetInstance()->Initialize();
    // 音声再生
    SoundPlayer::GetInstance()->Initialize(SoundLoader::GetInstance());
    // SRVマネージャーの初期化
    srvManager = new SrvManager();
    srvManager->Initialize(dxCommon);
    // ImGuiマネージャの初期化
    ImGuiManager::GetInstance()->Initialize(winApp.get(), dxCommon, srvManager);
    // テクスチャマネージャーの初期化
    TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
    // 3Dモデルマネージャの初期化
    ModelManager::GetInstance()->Initialize(dxCommon);

#pragma region 基盤システムの初期化

    // 入力の初期化
    Input::GetInstance()->Initialize(winApp.get());

    // スプライト共通部の初期化
    SpriteCommon::GetInstance()->Initialize(dxCommon);

    // 3Dオブジェクト共通部の初期化
    Object3dCommon::GetInstance()->Initialize(dxCommon);

#pragma endregion 基盤システムの初期化
}

void Framework::Update() {
    // Windowのメッセージ処理
    if (winApp->ProcessMessage()) {
        // ゲームループを抜ける
        endRequst_ = true;
    }
    // 入力の更新
    Input::GetInstance()->Update();
    // ImGuiの受付開始
    ImGuiManager::GetInstance()->Begin();
    // シーンマネージャの更新処理
    SceneManager::GetInstance()->Update();
}

void Framework::Draw() {
    //  描画用のDescriptorHeapの設定
    srvManager->PreDraw();
    //  DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
    dxCommon->PreDraw();
    // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
    Object3dCommon::GetInstance()->Commondrawing();
    // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
    SpriteCommon::GetInstance()->Commondrawing();
}