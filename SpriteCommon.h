#pragma once
#include"DirectXCommon.h"

class SpriteCommon
{
public: // メンバ関数
	// 初期化
	void Initialize(DirectXCommon* dxCommon);
	// 共通描画設定
	void Commondrawing();
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
public:
	// getter
	DirectXCommon* GetDxCommon() const { return  dxCommon_; }
};