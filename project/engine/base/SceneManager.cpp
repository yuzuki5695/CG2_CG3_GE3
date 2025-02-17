#include "SceneManager.h"
#include <cassert>

SceneManager* SceneManager::instance = nullptr;

SceneManager* SceneManager::GetInstance() {
	if (instance == nullptr) {
		instance = new SceneManager;
	}
	return instance;
}

void SceneManager::Finalize() {
	// 最後のシーンの終了と解放
	scene_->Finalize();
	delete scene_;
	scene_ = nullptr;
	delete instance;
	instance = nullptr;
}

void SceneManager::Update() {
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

void SceneManager::Draw() {
	scene_->Draw();
}

void SceneManager::ChangeScene(const std::string& sceneName) {
	assert(sceneFactory_);
	assert(nextScene_ == nullptr);

	//次シーン生成
	nextScene_ = sceneFactory_->CreateScene(sceneName);
}