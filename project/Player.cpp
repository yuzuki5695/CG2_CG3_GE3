#include "Player.h"
#include"Input.h"
#include"Enemy.h"
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"

Player::~Player() {
	if (object3d_ != nullptr) {
		delete object3d_;
		object3d_ = nullptr;
	}
}

void Player::OnCollision() { isDead_ = true; }

void Player::Initialize(Object3dCommon* object3dCommon, const std::string& filename, Input* input) {
	if (object3d_ == nullptr) {
		object3d_ = new Object3d;
	}

	this->input_ = input;

	// プレイヤーの3Dオブジェクト初期化
	object3d_->Initialize(object3dCommon);
	object3d_->SetModel(filename);

	transform.scale = object3d_->GetScale();
	transform.rotate = object3d_->GetRotate();
	transform.translate = object3d_->GetTranslate();

	transform = { 1.0f,1.0f,1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,2000.0f };

	// プレイヤーの初期位置を設定
	object3d_->SetScale(transform.scale);
	object3d_->SetRotate(transform.rotate);
	object3d_->SetTranslate(transform.translate);
}

void Player::Update() {
	object3d_->Update();

	if (input_->Pushkey(DIK_D)) {
		transform.translate.x += 0.05f;
	}
	if (input_->Pushkey(DIK_A)) {
		transform.translate.x -= 0.05f;
	}

	if (input_->Pushkey(DIK_W)) {
		transform.translate.y += 0.05f;
	}

	if (input_->Pushkey(DIK_S)) {
		transform.translate.y -= 0.05f;
	}

	// プレイヤーの位置を設定
	object3d_->SetScale(transform.scale);
	object3d_->SetRotate(transform.rotate);
	object3d_->SetTranslate(transform.translate);

}

void Player::Draw() {
	object3d_->Draw();
}

// プレイヤーのAABBを取得するメソッド
AABB Player::GetAABB() {
	AABB box;
	// プレイヤーの位置とスケールを基にAABBを計算
	box.left = transform.translate.x - (object3d_->GetScale().x * 0.5f);
	box.right = transform.translate.x + (object3d_->GetScale().x * 0.5f);
	box.top = transform.translate.y - (object3d_->GetScale().y * 0.5f);
	box.bottom = transform.translate.y + (object3d_->GetScale().y * 0.5f);
	return box;
}

// AABB同士の衝突判定
bool CheckCollisionAABB(const AABB& box1, const AABB& box2) {
	// X軸方向での重なりを確認
	bool overlapX = (box1.left < box2.right) && (box1.right > box2.left);

	// Y軸方向での重なりを確認
	bool overlapY = (box1.top < box2.bottom) && (box1.bottom > box2.top);

	// 両方の条件を満たせば衝突
	return overlapX && overlapY;
}

// プレイヤーと敵の衝突判定
bool Player::CheckCollisionWithEnemy(Enemy* enemy) {
	// プレイヤーと敵のAABBを取得
	AABB playerBox = this->GetAABB();  // プレイヤーのAABB
	AABB enemyBox = enemy->GetAABB();   // 敵のAABB

	// AABB同士の衝突をチェック
	return CheckCollisionAABB(playerBox, enemyBox);
}