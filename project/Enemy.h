#pragma once
#include "Object3dCommon.h"
#include "Object3d.h"

class Enemy
{
public:
public: // メンバ関数
	~Enemy(); // デストラクタを追加
	// 初期化
	void Initialize(Object3dCommon* object3dCommon, const std::string& filename);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();

	/// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision();

	AABB GetAABB();

private: // メンバ変数
	Object3d* object3d_ = nullptr;
	Transform transform{};
	// デスフラグ
	bool isDead_ = false;
	uint32_t movePattern = 0;
	Vector3 moveSpeed = { 0.05f,0.05f,0.05f };  // 移動スピード
	// 現在の位置を取得
	Vector3 position = GetTranslate();
	// 移動方向を制御するフラグ
	bool moveUp = true; // 初期値として上方向に移動する設定
public:
	// getter
	const Vector3& GetScale() const { return transform.scale; }
	const Vector3& GetRotate() const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }
	const bool GetisDead() const { return isDead_; }

	// setter
	void SetScale(const Vector3& scale) { this->transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { this->transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { this->transform.translate = translate; }
	void SetisDead(const bool& isDead) { this->isDead_ = isDead; }
	void SetMovePattern(uint32_t pattern) { movePattern = pattern; }
};