#include "Enemy.h"

Enemy::~Enemy() {
	if (object3d_ != nullptr) {
		delete object3d_;
		object3d_ = nullptr;
	}
}

void Enemy::OnCollision() { isDead_ = true; }


void Enemy::Initialize(Object3dCommon* object3dCommon, const std::string& filename) {
	if (object3d_ == nullptr) {
		object3d_ = new Object3d;
	}

	// オブジェクト初期化
	object3d_->Initialize(object3dCommon);
	object3d_->SetModel(filename);

	transform.scale = object3d_->GetScale();
	transform.rotate = object3d_->GetRotate();
	transform.translate = object3d_->GetTranslate();

	transform = { 1.0f,1.0f,1.0f,0.0f,0.0f,0.0f,5.0f,0.0f,0.0f };

	// 初期位置を設定
	object3d_->SetScale(transform.scale);
	object3d_->SetRotate(transform.rotate);
	object3d_->SetTranslate(transform.translate);
}

void Enemy::Update() {
	object3d_->Update();

	// 位置を設定
	object3d_->SetScale(transform.scale);
	object3d_->SetRotate(transform.rotate);
	object3d_->SetTranslate(transform.translate);
}

void Enemy::Draw() {
	if (!isDead_) {
		object3d_->Draw();
	}
}

AABB Enemy::GetAABB() {
	AABB box;
	// プレイヤーの位置とスケールを基にAABBを計算
	box.left = transform.translate.x - (object3d_->GetScale().x * 0.5f);
	box.right = transform.translate.x + (object3d_->GetScale().x * 0.5f);
	box.top = transform.translate.y - (object3d_->GetScale().y * 0.5f);
	box.bottom = transform.translate.y + (object3d_->GetScale().y * 0.5f);
	return box;
}