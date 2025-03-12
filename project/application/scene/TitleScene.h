#pragma once
#include<Sprite.h>
#include<Object3d.h>
#include<Model.h>
#include<SoundPlayer.h>
#include<BaseScene.h>

// ゲームプレイシーン
class TitleScene : public BaseScene
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
    std::shared_ptr <Sprite> sprite = nullptr;

    /*-----ファイルデータ----*/
    // Texture
    std::string TexturePath01;
    /*-----ファイルデータ----*/

};