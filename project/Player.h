#pragma once
#include "Object3dCommon.h"
#include "Object3d.h"

class Input;
class Enemy;

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

	/// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision();

	AABB GetAABB();

private: // メンバ変数
	Object3d* object3d_ = nullptr;
	Input* input_ = nullptr;
	Transform transform{};
	// デスフラグ
	bool isDead_ = false;
	// プレイヤーの移動スピード
	const float moveSpeed = 0.1f;
	const float dashSpeed = 0.2f; // ダッシュスピード
	Vector3 lastDirection = { 0.0f, 0.0f, 0.0f }; // 最後に押した方向を記録する変数

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