#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<wrl.h>
#include"WinApp.h"
#include<array>
#include<dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

// Directx基盤
class DirectXCommon
{
public: // メンバ関数

	// 初期化
	void Initialize(WinApp* winApp);

	/// <summary>
	/// デスクリプタヒープを生成する
	/// </summary>
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	/// <summary>
	/// SRV指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRV指定番号のGPUディスクリプタハンドルの取得をする
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

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
	
	/// <summary>
	/// 指定番号のCPUディスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index);

	/// <summary>
	/// 指定番号のGPUディスクリプタハンドルの取得をする
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index);
	
	// リソース
	Microsoft::WRL::ComPtr <ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr <ID3D12Device>& device, int32_t width, int32_t heigth);

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
	// 深度バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> depthbufferresource;
	// RTV用のヒープでディスクリプタ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap;
	// SRV用のヒープでディスクリプタ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap;
	// DSV用のヒープでディスクリプタ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap;
	// 各DescriptorSizeを取得する
	uint32_t descriptorsizeSRV;
	uint32_t descriptorsizeRTV;
	uint32_t descriptorsizeDSV;
	// スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle;
	//RTVを2つ作るのでディスクリプタハンドルを2つ用意
	const uint32_t rtvHandlenum = 2;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// フェンスの生成
	Microsoft::WRL::ComPtr <ID3D12Fence> fence = nullptr;
	// ビューポート
	D3D12_VIEWPORT viewport{};
	// シザー短形
	D3D12_RECT scissorRect{};
	// DXCコンパイラの初期化
	Microsoft::WRL::ComPtr <IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr <IDxcCompiler3> dxcCompiler = nullptr;
	Microsoft::WRL::ComPtr <IDxcIncludeHandler> includeHandler = nullptr;
};