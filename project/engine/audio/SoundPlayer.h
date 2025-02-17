#pragma once
#include "SoundLoader.h"

// 音声再生
class SoundPlayer {
public: // メンバ関数
    // 初期化
    void Initialize(SoundLoader* soundLoader);
    // 音声再生
    void SoundPlayWave(const SoundData& soundData, bool loop);
    // 音声データ解放
    void SoundUnload(SoundData* soundData);
private:
    SoundLoader* soundLoader_ = nullptr;
};