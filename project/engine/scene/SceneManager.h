#pragma once
#include<BaseScene.h>

// シーン管理
class SceneManager
{
private:

	static SceneManager* instance;

	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(SceneManager&) = delete;
	SceneManager& operator=(SceneManager&) = delete;
public: // メンバ関数
	// シングルトンインスタンスの取得
	static SceneManager* GetInstance();
	// 終了
	void Finalize();
	// 毎フレーム更新
	void Update();
	// 描画
	void Draw();
private: // メンバ変数
	// 今のシーン(実行中シーン)
	BaseScene* scene_ = nullptr;
	// 次のシーン
	BaseScene* nextScene_ = nullptr;
public:
	// 次シーン予約
	void SetNextScene(BaseScene* nextScene) { nextScene_ = nextScene; }
};