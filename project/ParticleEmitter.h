#pragma once
#include <Vector3.h>
#include <string>

// パーティクル発生器
class ParticleEmitter
{
public:
	//ほとんどのメンバ変数をコンストラクタの引数として受け取り、メンバ変数に代入する
	ParticleEmitter(
		const Vector3& position,

		const float lifetime,
		const float currentTime,
		const uint32_t count,
		const std::string& name

	);


	void Update();

	void Emit();


private: // メンバ変数
	// 座標
	Vector3 position_;






};

