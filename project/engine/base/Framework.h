#pragma once
#include<Input.h>
#include<DirectXCommon.h>
#include<ResourceObject.h>
#include<D3DResourceLeakChecker.h>
#include<TextureManager.h>
#include<SpriteCommon.h>
#include<Object3dCommon.h>
#include<ModelCommon.h>
#include<ModelManager.h>
#include<Camera.h>
#include<SrvManager.h>
#include<ImGuiManager.h>
#include<SoundPlayer.h>

// ゲーム全体
class Framework
{
public:// メンバ関数
	// デストラクタ
	virtual ~Framework() = default;

	// 初期化
	virtual void Initialize();
	// 終了
	virtual void Finalize();
	// 毎フレーム更新
	virtual void Update();
	// 描画
	virtual void Draw() = 0;

	// 実行
	void Run();

private: // メンバ変数
	// ポインタ
	Input* input = nullptr;
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;
	ModelCommon* modelCommon = nullptr;
	SrvManager* srvManager = nullptr;
	Camera* camera = nullptr;
	ImGuiManager* imGuiManager = nullptr;
	SoundLoader* soundLoader = nullptr;
	SoundPlayer* soundPlayer = nullptr;

	// カメラの現在の位置と回転を取得
	Vector3 Cameraposition;
	Vector3 Camerarotation;

	// ゲーム終了フラグ
	bool endRequst_ = false;
public:
	// getter 
	Input* GetInput() const { return input; }
	WinApp* GetWinApp() const { return winApp; }
	DirectXCommon* GetDirectXCommon() const { return dxCommon; }
	SpriteCommon* GetSpriteCommon() const { return spriteCommon; }
	Object3dCommon* GetObject3dCommon() const { return object3dCommon; }
	Camera* GetCamera() const { return camera; }
	SrvManager* GetSrvManager() const { return srvManager; }
	ImGuiManager* GetImGuiManager() const { return imGuiManager; }
	SoundLoader* GetSoundLoader() const { return soundLoader; }
	SoundPlayer* GetSoundPlayer() const { return soundPlayer; }
	// 終了フラグのチェック
	virtual bool IsEndRequst() { return  endRequst_; }
};