#pragma once
#include<xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include<fstream>
#include <wrl.h>

// チャンクヘッダ
struct ChunkHeader
{
    char id[4]; // チャンク毎のID
    int32_t size; // チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader
{
    ChunkHeader chunk; // "RIFF"
    char type[4]; // "WAVE"
};

// FMTチャンク
struct FormatChunk
{
    ChunkHeader chunk; // "fmt"
    WAVEFORMATEX fmt; // 波形フォーマット
};

// 音声データ
struct SoundData
{
    // 波形フォーマット
    WAVEFORMATEX wfex;
    // バッファの先頭アドレス
    BYTE* pBuffer;
    // バッファのサイズ
    unsigned int bufferSize;
};

class SoundLoader
{
public: // メンバ関数
    // デストラクタ
    ~SoundLoader() { Finalize(); }
    // 初期化
    void Initialize();
    // wavファイル読み込み
    static SoundData SoundLoadWave(const char* filename);
    // 音声データリセット
    void Finalize();
private:// メンバ変数
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
    IXAudio2MasteringVoice* masterVoice;
public:
    // getter
    Microsoft::WRL::ComPtr<IXAudio2> GetIXAudio2() const { return xAudio2; }
};