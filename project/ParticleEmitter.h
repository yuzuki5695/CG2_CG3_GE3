#pragma once
#include <Vector3.h>
#include <string>

// パーティクル発生器
class ParticleEmitter
{
public:
	//ほとんどのメンバ変数をコンストラクタの引数として受け取り、メンバ変数に代入する
	ParticleEmitter(
		const std::string& name,
		const Vector3& position,
		const uint32_t count,
		const float time,
		const float Remainingtime
	);


	void Update();

	void Emit();


private: // メンバ変数
	// 名前
	std::string name_;
	// 座標
	Vector3 position_;
	// 時間
	float time_;
	// 残り時間
	float Remainingtime_;
	// count
	uint32_t count_;
};

