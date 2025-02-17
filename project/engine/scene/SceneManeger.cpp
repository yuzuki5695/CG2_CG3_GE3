#include "SceneManeger.h"

SceneManeger* SceneManeger::instance = nullptr;

SceneManeger* SceneManeger::GetInstance() {
	if (instance == nullptr) {
		instance = new SceneManeger;
	}
	return instance;
}

void SceneManeger::Finalize() {
	// 最後のシーンの終了と解放
	scene_->Finalize();
	delete scene_;
	delete instance;
	instance = nullptr;
}


void SceneManeger::Update() {
	// TODO:シーンの切り替え機構


	// 次のシーンの予約があるなら
	if (nextScene_) {
		// 旧シーンの終了
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}

		// シーン切り替え
		scene_ = nextScene_;
		nextScene_ = nullptr;
		// シーンマネージャをセット 
		scene_->SetSceneManeger(this);
		// 次のシーンを初期化する
		scene_->Initialize();
	}

	// 実行中シーンを更新する
	scene_->Update();
}

void SceneManeger::Draw() {
	scene_->Draw();
}