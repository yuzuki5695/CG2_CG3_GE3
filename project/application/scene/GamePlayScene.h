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
    // Sprite
    std::shared_ptr <Sprite> sprite;
    // Object3d
    std::unique_ptr <Model> model;
    std::shared_ptr<Object3d> object;
    /*-----ファイルデータ----*/
    // Texture
    std::string TexturePath01;
    std::string TexturePath02;
    std::string TexturePath03;
    // Model
    std::string ModelPath01;
    /*-----ファイルデータ----*/
};