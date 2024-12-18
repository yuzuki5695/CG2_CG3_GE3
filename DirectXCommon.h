#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<wrl.h>
#include"WinApp.h"

// Directx基盤
class DirectXCommon
{
public: // メンバ関数

	// 初期化
	void Initialize(WinApp* winApp);

private: // プライベートメンバ関数
	// デバイスの初期化
	void DebugInitialize();
	// コマンド関連の初期化
	void CommandInitialize();
	// スワップチェーンの生成
	void SwapChainGenerate();
	// 深度バッファの生成
	void CreateDepthStencilGenerate();
	// 各種でスクリプタヒープの生成
	void DescriptorHeapGenerate();
	// レンダーターゲットビューの初期化
	void RenderviewInitialize();
	// 深度ステルスビューの初期化
	void DepthstealthviewInitialization();
	// フェンスの初期化
	void FenceInitialize();
	// ビューポートの初期化
	void viewportInitialize();
	// シザリング矩形
	void scissorRectInitialize();
	// DXCコンパイラの生成
	void DxCompilerGenerate();
	// imguiの初期化
	void ImguiInitialize();


private: // メンバ変数
	// ポインタ
	WinApp* winApp_ = nullptr;
	// Devicex12デバイス
	Microsoft::WRL::ComPtr <ID3D12Device> device;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr <IDXGIFactory7> dxgiFactory;
	// コマンドアロケータ
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator = nullptr;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	// コマンドキュー
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue;
	// SwapChain(スワップチェーン)
	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain;
};