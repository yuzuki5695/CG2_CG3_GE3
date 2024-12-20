#include "DirectXCommon.h"
#include<cassert>
#include <format>
#include "Logger.h"
#include "StringUtility.h"
#include"externals/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
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
    hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp_->Gethwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
    assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateDepthStencilGenerate() {

    // 生成するResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = winApp_->kClientWidth; // Textureの幅
    resourceDesc.Height = winApp_->kClientHeight; // Textureの高さ
    resourceDesc.MipLevels = 1; // mipmapの数
    resourceDesc.DepthOrArraySize = 1; // 奥行　or 配列のTexture配列数
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
    resourceDesc.SampleDesc.Count = 1; // サンプリングカウント 1固定
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

    // 利用Heapの設定
    D3D12_HEAP_PROPERTIES heapProperies{};
    heapProperies.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAW上に作る

    // 深度値のクリア設定
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f (最大値)でクリア
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。 Resourceと合わせる

    //3. Resourceを生成する
    HRESULT hr = device->CreateCommittedResource(
        &heapProperies, //Heapの設定
        D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定。特になし。
        &resourceDesc, //Resourceの設定
        D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値を書き込む状態にしておく
        &depthClearValue, //Clear最適値
        IID_PPV_ARGS(&depthbufferresource)); //作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));
}

void DirectXCommon::DescriptorHeapGenerate() {
    /*----------------------------------------------------------------------------*/
    /*---------------------------DescriptorHeap-----------------------------------*/
    /*----------------------------------------------------------------------------*/

    // RTV用のヒープでディスクリプタの数は2。RTVはshader内で触るものではないので、ShaderVisibleはfalse
    rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
    // SRV用のヒープでディスクリプタの数は128.RTVはshader内で触るものなので、ShaderVisibleはtrue
    srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
    // DSV用のヒープでディスクリプタの数は1。DSVはshader内で触るものではないので、ShaderVisibleはfalse
    dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

    // DescriptorSizeを取得する
    descriptorsizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    descriptorsizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    descriptorsizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void DirectXCommon::RenderviewInitialize() {

    HRESULT hr;

    //SwapChainからResourceを引っ張ってくる
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
    //上手く取得できなければ起動できない
    assert(SUCCEEDED(hr));
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
    assert(SUCCEEDED(hr));
    
    /*-----------------------------------------------------------*/
    /*--------------------------RTVの設定--------------------------*/
    /*------------------------------------------------------------*/
    
    //RTVの設定
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGB2変換して書き込む
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャとして読み込む
    //ディスクリプタの先頭を取得する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorsizeRTV, 0);
    // ハンドルの数だけ作成する
    for (uint32_t i = 0; i < rtvHandlenum; ++i) {
        rtvHandles[i] = rtvStartHandle;
        device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, rtvHandles[i]);
    }
}


void DirectXCommon::DepthstealthviewInitialization() {

    /*------------------------------------------------------------*/
    /*--------------------------DSVの設定--------------------------*/
    /*------------------------------------------------------------*/

    // DepthStencilTextureをウインドウのサイズで作成
    Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device,WinApp::kClientWidth, WinApp::kClientHeight);

    // DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format。基本的にはResource合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2dTexture 
    // DSVDescの先頭にDSVを作る
    device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    // Depthの機能を有効化する
    depthStencilDesc.DepthEnable = true;
    // 書き込みする
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    // 比較関数はLessEqual。つまり、近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void DirectXCommon::FenceInitialize() {

    HRESULT hr;

    //初期値0でFenceを作る
    uint64_t fenceValue = 0;
    hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));
    //FenceのSignalを待つためのイベントを作成する
    HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);
}

void DirectXCommon::viewportInitialize() {
    //クライアント領域のサイズと一緒にして画面全体に表示
    viewport.Width = WinApp::kClientWidth;
    viewport.Height = WinApp::kClientHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
}

void DirectXCommon::scissorRectInitialize() {
    // 基本的にビューポートと同じ矩形が構成されるようにする
    scissorRect.left = 0;
    scissorRect.right = WinApp::kClientWidth;
    scissorRect.top = 0;
    scissorRect.bottom = WinApp::kClientHeight;
}

void DirectXCommon::DxCompilerGenerate() {

    HRESULT hr;

    // dxCompilerを初期化
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));
    //現時点でincludeはしないが、includeに対応するための設定を行っていく
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));
}

void DirectXCommon::ImguiInitialize() {

    //-----------------------------//
    //-------ImGuiの初期化-----------//
    //-----------------------------//
  /*  IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(winApp_->Gethwnd());
    ImGui_ImplDX12_Init(device.Get(),
        swapChainDesc.BufferCount,
        rtvDesc.Format,
        srvDescriptorHeap.Get(),
        srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());*/
}












ComPtr <ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible){
    //ディスクリプタヒープの生成
    ComPtr <ID3D12DescriptorHeap> descriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = heapType;
    descriptorHeapDesc.NumDescriptors = numDescriptors;
    descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
    //ディスクリプタヒープが作れなかったので起動できない
    assert(SUCCEEDED(hr));
    return descriptorHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index) {

    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorsize * index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorsize * index);
    return handleGPU;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) {

    return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorsizeSRV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index) {

    return GetGPUDescriptorHandle(srvDescriptorHeap, descriptorsizeSRV, index);
}

ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(ComPtr <ID3D12Device>& device, int32_t width, int32_t heigth) {
    // 生成するResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = width; // Textureの幅
    resourceDesc.Height = heigth; // Textureの高さ
    resourceDesc.MipLevels = 1; // mipmapの数
    resourceDesc.DepthOrArraySize = 1; // 奥行　or 配列のTexture配列数
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
    resourceDesc.SampleDesc.Count = 1; // サンプリングカウント 1固定
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

    // 利用Heapの設定
    D3D12_HEAP_PROPERTIES heapProperies{};
    heapProperies.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAW上に作る

    // 深度値のクリア設定
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f (最大値)でクリア
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。 Resourceと合わせる


    //3. Resourceを生成する
    ComPtr <ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperies, //Heapの設定
        D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定。特になし。
        &resourceDesc, //Resourceの設定
        D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値を書き込む状態にしておく
        &depthClearValue, //Clear最適値
        IID_PPV_ARGS(&resource)); //作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));
    return resource;
}
