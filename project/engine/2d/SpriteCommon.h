#pragma once
#include"DirectXCommon.h"

// スプライト共通部
class SpriteCommon
{
private:
	static SpriteCommon* instance;

	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(SpriteCommon&) = delete;
	SpriteCommon& operator=(SpriteCommon&) = delete;
public: // メンバ関数
	// シングルトンインスタンスの取得
	static SpriteCommon* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* dxCommon);
	// 共通描画設定
	void Commondrawing();

	DirectXCommon* GetDxCommon() const { return  dxCommon_; }
private:
	// ルートシグネチャの生成
	void RootSignatureGenerate();
	// グラフィックスパイプラインの生成
	void GraphicsPipelineGenerate();
private:
	// ポインタ
	DirectXCommon* dxCommon_;
	// RootSignature
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
};