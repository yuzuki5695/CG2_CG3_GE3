#include "DirectXCommon.h"
#include<cassert>
#include "Logger.h"
#include "StringUtility.h"
#include <format>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

void DirectXCommon::Initialize(WinApp* winApp){
    // NULL検出
    assert(winApp);
    // メンバ変数に記録
    this->winApp_ = winApp;

	// デバイスの初期化
	DebugInitialize();
	// コマンド関連の初期化
	CommandInitialize();
	// スワップチェーンの生成
	SwapChainGenerate();
	// 深度バッファの生成
	CreateDepthStencilGenerate();
	// 各種でスクリプタヒープの生成
	DescriptorHeapGenerate();
	// レンダーターゲットビューの初期化
	RenderviewInitialize();
	// 深度ステルスビューの初期化
	DepthstealthviewInitialization();
	// フェンスの初期化
	FenceInitialize();
	// ビューポートの初期化
	viewportInitialize();
	// シザリング矩形
	scissorRectInitialize();
	// DXCコンパイラの生成
	DxCompilerGenerate();
	// imguiの初期化
	ImguiInitialize();
}

void DirectXCommon::DebugInitialize() {

    HRESULT hr;

    ///---------------------------------------------------------------------///
    ///-----------------------デバックレイヤーをオン----------------------------///
    ///---------------------------------------------------------------------///
#ifdef _DEBUG
    ComPtr <ID3D12Debug1> debugController = nullptr;

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        //デバッグレイヤーを有効化
        debugController->EnableDebugLayer();
        //さらにGPU側でもチェックを行えるようにする
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif // _DEBUG

    ///---------------------------------------------------------------------///
    ///----------------------IDXGIのファクトリー生成---------------------------///
    ///---------------------------------------------------------------------///

    // HRESULTはWindows系のエラーコードであり、
    // 関数が成功したかどうかをSUCCEEDEDマクロで判定できる
    hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    //初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が
    //多いのでassertにする
    assert(SUCCEEDED(hr));

    ///---------------------------------------------------------------------///
    ///--------------------------アダプターの列挙------------------------------///
    ///---------------------------------------------------------------------///
    //仕様するアダプター用の変数。最初にnullptrを入れておく
    ComPtr <IDXGIAdapter4> useAdapter = nullptr;
    //良い順にアダプターを頼む
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
        DXGI_ERROR_NOT_FOUND; ++i) {
        //アダプターの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = useAdapter->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr));//取得できないのは一大事
        //ソフトウェアアダプターでなければ採用
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            //採用したアダプタの情報をログに出力。wstringの方なので注意
            Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
            break;
        }
        useAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
    }
    //適切なアダプタが見つからないので起動できない
    assert(useAdapter != nullptr);

    ///---------------------------------------------------------------------///
    ///---------------------------デバイスの生成------------------------------///
    ///---------------------------------------------------------------------///

    //機能レベルとログ出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
    };
    const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
    //高い順に生成できるか試す
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        //採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
        //指定した機能レベルでデバイスが生成できたか確認
        if (SUCCEEDED(hr)) {
            //生成できたのでログ出力を行ってループを抜ける
            Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
            break;
        }
    }
    //デバイスの生成がうまくいかなかったので起動できない
    assert(device != nullptr);
    Logger::Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

    ///---------------------------------------------------------------------///
    ///-------------------------エラー時にブレ―ク-----------------------------///
    ///--------------------------------------------------------------------///
#ifdef _DEBUG
    ID3D12InfoQueue* infoQueue = nullptr;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        //ヤバイエラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        //エラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        //警告時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        //抑制するメッセージのID
        D3D12_MESSAGE_ID denyIds[] = {
            //Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
            //https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        //抑制するレベル
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        //指定したメッセージの表示を抑制
        infoQueue->PushStorageFilter(&filter);

        //解放
        infoQueue->Release();
    }
#endif
}

void DirectXCommon::CommandInitialize() {

    HRESULT hr;

    ///---------------------------------------------------------------------///
    ///------------------------コマンドキューを生成する-------------------------///
    ///---------------------------------------------------------------------///
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
    //コマンドキューの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    ///---------------------------------------------------------------------///
    ///---------------------コマンドアロケータを生成する-------------------------///
    ///---------------------------------------------------------------------///
    //コマンドアロケーターを生成する
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    //コマンドアロケータの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    ///---------------------------------------------------------------------///
    ///------------------------コマンドリストを生成する-------------------------///
    ///---------------------------------------------------------------------///
    //コマンドリストを生成する
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    //コマンドリストの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));
}

void DirectXCommon::SwapChainGenerate() {

    HRESULT hr;

    ///---------------------------------------------------------------------///
    ///--------------SwapChain(スワップチェーン)を設定する----------------------///
    ///---------------------------------------------------------------------///
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = WinApp::kClientWidth;//画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Height = WinApp::kClientHeight;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
    swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2;//ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに移したら、中身居を破棄
    
    ///---------------------------------------------------------------------///
    ///--------------SwapChain(スワップチェーン)を生成する----------------------///
    ///---------------------------------------------------------------------///
    hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->Gethwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
    assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateDepthStencilGenerate() {



}