#include "ParticleEmitter.h"
#include<ParticleManager.h>

ParticleEmitter::ParticleEmitter(const Vector3& position, const float lifetime, const float currentTime, const uint32_t count, const std::string& name, const Vector3& Velocity)
{
	position_ = position;//位置
	frequency = lifetime;//寿命
	frequencyTime = currentTime;//現在の寿命
	this->count = count;//count
	name_ = name;//名前
	velocity_ = Velocity; // 風の強さ
}

void ParticleEmitter::Update()
{
	//// 時間を進める
	//frequencyTime += 1.0f / 60.0f;

	//// パーティクルグループを取得
	//ParticleManager::ParticleGroup& group = ParticleManager::GetInstance()->GetGroup(name_);

	//// 一定時間経過で発生処理
	//if (frequencyTime >= frequency) {
	//	size_t currentCount = group.particles.size(); // 現在のパーティクル数
	//	const uint32_t maxCount = ParticleManager::; // 最大数（仮定）

	//	// 上限未満なら発生
	//	if (currentCount < maxCount) {
	//		// 発生可能な数だけ生成
	//		uint32_t emitCount = static_cast<uint32_t>(std::min<size_t>(count, maxCount - currentCount));
	//		ParticleManager::GetInstance()->Emit(name_, position_, emitCount);
	//	}

	//	// 時間をリセット
	//	frequencyTime = 0.0f;
	//}
}

void ParticleEmitter::Emit()
{
	//パーティクルを発生
	ParticleManager::GetInstance()->Emit(name_, position_, count);
}