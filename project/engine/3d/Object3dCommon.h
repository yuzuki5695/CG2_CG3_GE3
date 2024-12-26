#pragma once
#include"DirectXCommon.h"
#include"Camera.h"

// 3Dオブジェクト共通部
class Object3dCommon
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
	Camera* defaultCamera = nullptr;
	// RootSignature
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
public:
	// setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	// getter
	DirectXCommon* GetDxCommon() const { return  dxCommon_; }
	Camera* GetDefaultCamera() const { return defaultCamera; }
};