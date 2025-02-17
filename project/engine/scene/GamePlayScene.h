#pragma once
#include<Sprite.h>
#include<Object3d.h>
#include<Model.h>
#include<SoundPlayer.h>

// ゲームプレイシーン
class GamePlayScene
{
public: // メンバ関数

	void Initialize();
	// 終了
	void Finalize();
	// 毎フレーム更新
	void Update();
	// 描画
	void Draw();
private: // メンバ変数
    // オブジェクトデータ
// Sprite
    Sprite* sprite = nullptr;
    // Object3d
    Model* model = nullptr;
    Object3d* object3d = nullptr;
    /*-----ファイルデータ----*/
    // Texture
    std::string TexturePath01;
    std::string TexturePath02;
    // Model
    std::string ModelPath01;
    std::string ModelPath02;
    // 音声
    SoundData soundData;
    /*-----ファイルデータ----*/

    // 音声プレイフラグ
    uint32_t soundfige;
};