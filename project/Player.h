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
public:
	// getter
	const Vector3& GetScale() const { return transform.scale; }
	const Vector3& GetRotate() const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }

	// setter
	void SetScale(const Vector3& scale) { this->transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { this->transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { this->transform.translate = translate; }
};