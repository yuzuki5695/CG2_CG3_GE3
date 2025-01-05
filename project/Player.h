#pragma once
#include "Object3dCommon.h"
#include "Object3d.h"

class Input;

class Player
{
public:
public: // メンバ関数
	~Player(); // デストラクタを追加
	// 初期化
	void Initialize(Object3dCommon* object3dCommon, const std::string& filename, Input* input);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();


private: // メンバ変数
	Object3d* object3d_ = nullptr;
	Input* input_ = nullptr;
	Transform transform{};
};

