#pragma once
#include<Sprite.h>
#include<Object3d.h>
#include<Model.h>
#include<SoundPlayer.h>
#include<BaseScene.h>

// ゲームプレイシーン
class GamePlayScene : public BaseScene
{
public: // メンバ関数
    // 初期化
    void Initialize() override;
    // 終了
    void Finalize() override;
    // 毎フレーム更新
    void Update() override;
    // 描画
    void Draw() override;
private: // メンバ変数
    // オブジェクトデータ
    // camera
    std::unique_ptr <Camera> camera = nullptr;
    // Sprite
    std::unique_ptr<Sprite> sprite = nullptr;
    // Object3d
    std::unique_ptr <Object3d> object3d = nullptr;
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

    // カメラの現在の位置と回転を取得
    Vector3 Cameraposition;
    Vector3 Camerarotation;
};