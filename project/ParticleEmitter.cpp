#include "ParticleEmitter.h"
#include "ParticleManager.h"

ParticleEmitter::ParticleEmitter(const std::string& name, const Vector3& position, const uint32_t count, const float time, const float Remainingtimee) {
	name_ = name;
	position_ = position;
	count_ = count;
	time_ = time;
	Remainingtime_ = Remainingtimee;
}

void ParticleEmitter::Update()
{
	// 時刻を進める
	Remainingtime_ += 1.0f / 60.0f;
	// 発生頻度より大きいなら発生
	if (Remainingtime_ > time_) {
		// パーティクル発生
		ParticleManager::GetInstance()->Emit(name_, position_, count_);
		// 時間をリセット
		Remainingtime_ = 0.0f;
	}
}

void ParticleEmitter::Emit()
{
	//パーティクルを発生
	ParticleManager::GetInstance()->Emit(name_, position_, count_);
}