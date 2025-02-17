#pragma once

// 前方宣言 
class SceneManeger;

// シーン基底クラス
class BaseScene
{
public:// メンバ関数
	// デストラクタ
	virtual ~BaseScene() = default;

	// 初期化
	virtual void Initialize() = 0;
	// 終了
	virtual void Finalize() = 0;
	// 毎フレーム更新
	virtual void Update() = 0;
	// 描画
	virtual void Draw() = 0;
private:
	// シーンマネージャ（借りてくる）
	SceneManeger* sceneManeger_ = nullptr;
public:// メンバ関数
	virtual void SetSceneManeger(SceneManeger* sceneManeger) { sceneManeger_ = sceneManeger; }
	SceneManeger* GetSceneManeger() { return sceneManeger_; }
};