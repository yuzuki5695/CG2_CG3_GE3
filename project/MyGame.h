#pragma once
#include<MatrixVector.h>
#include<ResourceObject.h>
#include<Input.h>
#include<DirectXCommon.h>
#include<D3DResourceLeakChecker.h>
#include<Transform.h>
#include<TextureManager.h>
#include<SpriteCommon.h>
#include<Sprite.h>
#include<Object3dCommon.h>
#include<Object3d.h>
#include<ModelCommon.h>
#include<Model.h>
#include<ModelManager.h>
#include<Camera.h>
#include<SrvManager.h>
#include<ImGuiManager.h>
#include<SoundPlayer.h>

// ゲーム
class MyGame
{
public: // メンバ関数

    // 初期化
    void Initialize();
    // 終了
    void Finalize();
    // 毎フレーム更新
    void Update();
    // 描画
    void Draw();

private: // メンバ変数
    // ポインタ
    Input* input = nullptr;
    WinApp* winApp = nullptr;
    DirectXCommon* dxCommon = nullptr;
    SpriteCommon* spriteCommon = nullptr;
    Object3dCommon* object3dCommon = nullptr;
    ModelCommon* modelCommon = nullptr;
    SrvManager* srvManager = nullptr;
    ImGuiManager* imGuiManager = nullptr;
    SoundLoader* soundLoader = nullptr;
    SoundPlayer* soundPlayer = nullptr;

    // カメラ
    Camera* camera;
    Vector3 Cameraposition;
    Vector3 Camerarotation;

    // オブジェクトデータ
    // Sprite
    Sprite* sprite = nullptr;
    // Object3d
    Model* model = nullptr;
    Object3d* object3d = nullptr;

    /*-----ファイルデータ----*/
    // Texture
    std::string TexturePath01;
    std::string TexturePath02;
    // Model
    std::string ModelPath01;
    std::string ModelPath02;
    // 音声
    SoundData soundData;
    /*-----ファイルデータ----*/

    // 音声プレイフラグ
    uint32_t soundfige;

    // ゲーム終了フラグ
    bool endRequst_ = false;
public:
    // getter
    // 終了フラグのチェック
    bool IsEndRequst() { return  endRequst_; }
};