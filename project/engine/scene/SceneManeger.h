#pragma once
#include<BaseScene.h>

// シーン管理
class SceneManeger
{
public: // メンバ関数
	~SceneManeger();
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