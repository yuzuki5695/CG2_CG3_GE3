#pragma once
#include"DirectXCommon.h"

// 3Dオブジェクト共通部
class Object3dCommon
{
public: // メンバ関数
	// 初期化
	void Initialize(DirectXCommon* dxCommon);

private:
	// ポインタ
	DirectXCommon* dxCommon_;
	// ルートシグネチャの生成
	void RootSignatureGenerate();
	// グラフィックスパイプラインの生成
	void GraphicsPipelineGenerate();

public:
	// getter
	DirectXCommon* GetDxCommon() const { return  dxCommon_; }
};