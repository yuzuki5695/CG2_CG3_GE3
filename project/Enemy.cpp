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

	// 移動方法を切り替え
	switch (movePattern) {
	case 0: // 縦移動
		if (moveUp) {
			transform.translate.y += moveSpeed.y; // 上方向に移動
			if (transform.translate.y > 30.0f) { // 上限に達した場合
				// X座標をランダムに設定
				transform.translate.x = -30.0f + static_cast<float>(std::rand() % 61);
				// Y座標をリセット
				transform.translate.y = -50.0f + static_cast<float>(std::rand() % 21 - 30); // -50 ~ -30 の範囲
				// ランダムで移動速度を設定
				moveSpeed.y = 0.05f + static_cast<float>(std::rand()) / RAND_MAX * 0.05f;
				// 再移動時の方向を抽選 (true: 上移動, false: 下移動)
				moveUp = (std::rand() % 2 == 0);
			}
		} else {
			transform.translate.y -= moveSpeed.y; // 下方向に移動
			if (transform.translate.y < -50.0f) { // 下限に達した場合
				// X座標をランダムに設定
				transform.translate.x = -30.0f + static_cast<float>(std::rand() % 61);
				// Y座標をリセット
				transform.translate.y = 30.0f; // 上限のスタート地点
				// ランダムで移動速度を設定
				moveSpeed.y = 0.05f + static_cast<float>(std::rand()) / RAND_MAX * 0.05f;
				// 再移動時の方向を抽選 (true: 上移動, false: 下移動)
				moveUp = (std::rand() % 2 == 0);
			}
		}
		break;

	case 1: // 横移動
		if (transform.translate.x > 35.0f) {
			// Y座標をランダムに設定 (-25.0f ～ 25.0f)
			transform.translate.y = -25.0f + static_cast<float>(std::rand() % 51);
			// X座標をリセット (-80.0f ～ -20.0f)
			transform.translate.x = -80.0f + static_cast<float>(std::rand()) / RAND_MAX * 60.0f;
			// ランダムな移動速度 (0.05f ～ 0.1f)
			moveSpeed.x = 0.05f + static_cast<float>(std::rand()) / RAND_MAX * 0.05f;
		}

		// X軸方向に移動
		transform.translate.x += moveSpeed.x;
		break;

	case 2: // ジグザグ移動
	{
		static bool zigzagUp = true; // ジグザグ移動の方向フラグ

		// Y方向の移動制御
		if (transform.translate.y > 30.0f || transform.translate.y < -50.0f) {
			zigzagUp = !zigzagUp; // 方向を反転
			// ランダムな移動速度を設定 (0.05f ～ 0.1f)
			moveSpeed.y = 0.05f + static_cast<float>(std::rand()) / RAND_MAX * 0.05f;
		}

		// X方向の移動制御
		if (transform.translate.x > 35.0f || transform.translate.x < -80.0f) {
			// Y座標をランダムに設定 (-25.0f ～ 25.0f)
			transform.translate.y = -25.0f + static_cast<float>(std::rand() % 51);
			// X座標をリセット (-80.0f ～ -20.0f)
			transform.translate.x = -80.0f + static_cast<float>(std::rand()) / RAND_MAX * 60.0f;
			// ランダムな移動速度を設定 (0.05f ～ 0.1f)
			moveSpeed.x = 0.05f + static_cast<float>(std::rand()) / RAND_MAX * 0.05f;
		}

		// 方向に応じて移動を加算
		transform.translate.x += moveSpeed.x;
		transform.translate.y += (zigzagUp ? moveSpeed.y : -moveSpeed.y);
		break;
	}


	default:
		break;
	}
	
	transform.rotate.x += moveSpeed.x;
	transform.rotate.y += moveSpeed.y;
	transform.rotate.z += moveSpeed.z;

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