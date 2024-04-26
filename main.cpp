#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include<dxgidebug.h>
#include<dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

//�N���C�A���g�̈�̃T�C�Y
const uint32_t kClientWidth = 1280;
const uint32_t kClientHeight = 720;

void Log(const std::string& message)
{
	OutputDebugStringA(message.c_str());
}

std::wstring ConvertString(const std::string& str)
{
	if (str.empty())
	{
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0)
	{
		return std::wstring();
	}
	//string -> wstring
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str)
{
	if (str.empty())
	{
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0)
	{
		return std::string();
	}
	//wstring -> string
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

//�E�B���h�E�v���V�[�W��
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//���b�Z�[�W�ɉ������Q�[���ŗL�̏���
	switch (msg) {
		//�E�B���h�E�j��
	case WM_DESTROY:
		//OS�ɑ΂��āA�A�v���̏I����`����
		PostQuitMessage(0);
		return 0;
	}

	//�W���̃��b�Z�[�W�������s��
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


IDxcBlob* CompileShader(
	//Compiler����Shader�t�@�C���̃p�X
	const std::wstring& filePath,
	//Compiler�Ɏg�p����Profile
	const wchar_t* profile,
	//�������Ő����������̂�3��
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler) {
	//�V�F�[�_�[���R���p�C������|�����O�ɏo��
	Log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
	//hlsl�t�@�C����ǂ�
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//�ǂ߂Ȃ���Ύ~�߂�
	assert(SUCCEEDED(hr));
	//�ǂݍ��񂾃t�@�C���̓��e��ݒ�
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8�̕����R�[�h�ł��邱�Ƃ��m�F

	LPCWSTR arguments[] = {
		filePath.c_str(),
		L"-E",L"main",
		L"-T",profile,
		L"-Zi",L"-Qembed_debug",
		L"-Od",
		L"-Zpr"
	};

	//���ۂ�Shader���R���p�C��
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler,
		IID_PPV_ARGS(&shaderResult)
	);
	//�R���p�C���G���[�ł͂Ȃ�dxc���N���ł��Ȃ��Ȃǒv���I�ȏ�
	assert(SUCCEEDED(hr));
	//�x���E�G���[���ł��烍�O�ɏo���Ď~�߂�
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		//�x���E�G���[�_�����
		assert(false);
	}
	//�R���p�C�����ʂ�����s�p�̃o�C�i���������擾
	IDxcBlob* shadeBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shadeBlob), nullptr);
	assert(SUCCEEDED(hr));
	//�����������O���o��
	Log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));

	//�����g��Ȃ����\�[�X�����
	shaderSource->Release();
	shaderResult->Release();
	//���s�p�̃o�C�i����ԋp
	return shadeBlob;
};


































//Windows�A�v���ł̃G���g���[�|�C���g�imain�֐��j
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WNDCLASS wc{};

	//�E�B���h�E�v���V�[�W��
	wc.lpfnWndProc = WindowProc;

	//�E�B���h�E�N���X���i�Ȃ�ł������j
	wc.lpszClassName = L"CG2WindowClass";

	//�C���X�^���X�n���h��
	wc.hInstance = GetModuleHandle(nullptr);

	//�J�[�\��
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//�E�B���h�E�N���X�o�^
	RegisterClass(&wc);

	//�E�B���h�E�T�C�Y��\���\���̂ɃN���C�A���g�̈������
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	//�N���C�A���g�̈�����Ɏ��ۂ̃T�C�Y��wrc�ɂ���
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�̐���
	HWND hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);


#ifdef _DEBUG

	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//�f�o�b�N���C���[��L��������
		debugController->EnableDebugLayer();
		//�����GPU���ł��`�F�b�N���s���悤�ɂ���
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif 


	//�E�B���h�E��\��
	ShowWindow(hwnd, SW_SHOW);

	//DXGI�t�@�N�g���[�̐���
	IDXGIFactory7* dxgiFactory = nullptr;

	//�֐��������������ǂ�����SUCCEEDED�}�N������ł���
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	//�������̍��{�I�ȕ����ŃG���[���o���ꍇ,�v���O�������Ԉ���Ă��邩�A�ǂ��ɂ��ł��Ȃ��ꍇ������̂�assert����
	assert(SUCCEEDED(hr));

	//�g�p����A�_�v�^�p�̕ϐ��B�ŏ���nullptr�����Ă���
	IDXGIAdapter4* useAdapter = nullptr;

	//�A�_�v�^��ǂ����ɗ���
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference(i,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND, ++i;) {
		//�A�_�v�^�[�̏����擾
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));	//�擾�ł��Ȃ��͈̂�厖

		//�\�t�g�E�F�A�A�_�v�^�łȂ���΍̗p!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//�̗p�����A�_�v�^�̏������O�ɏo�́Bwstring�̖@�Ȃ̂Œ���
			Log(ConvertString(std::format(L"Use Adapater\n:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;	//�\�t�g�E�F�A�A�_�v�^�̏ꍇ�͌��Ȃ��������Ƃɂ���
	}

	//�K�؂ȃA�_�v�^��������Ȃ������̂ŋN���ł��Ȃ�
	assert(useAdapter != nullptr);

	ID3D12Device* device = nullptr;

	//�@�\���x���ƃ��O�o�͗p�̕�����
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };

	//�������ɐ����ł��邩����
	for (size_t i = 0; i < _countof(featureLevels); i++) {
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));

		//�w�肵���@�\���x���Ńf�o�C�X�������ł������m�F
		if (SUCCEEDED(hr))
		{
			//�����ł����̂Ń��O�o�͂��s�����[�v�𔲂���
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}

	//�f�o�C�X�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n");	//�����������̃��O���o��

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//���o�C�G���[���Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//�G���[���Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//�x�����Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		//���
		infoQueue->Release();

		//�}�����郁�b�Z�[�W��ID
		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//�}�����x��
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		//�w�肵�����b�Z�[�W�̕\����}��
		infoQueue->PushStorageFilter(&filter);

	}
#endif 

	//�R�}���h�L���[����
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commasndQueueDesc{};
	hr = device->CreateCommandQueue(&commasndQueueDesc, IID_PPV_ARGS(&commandQueue));
	//��������肭�������N���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//�R�}���h�A���P�[�^����
	ID3D12CommandAllocator* commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//��������肭�������N���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//�R�}���h���X�g����
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	//��������肭�������N���ł��Ȃ�
	assert(SUCCEEDED(hr));


	//�X���b�v�`�F�[������
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;
	swapChainDesc.Height = kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//�R�}���h�L���[�A�E�B���h�n���h���A�ݒ��n���Đ�������
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	//��������肭�������N���ł��Ȃ�
	assert(SUCCEEDED(hr));


	//�f�B�X�N���v�^�q�[�v�̐���
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	//��������肭�������N���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//SwapChain����Resource�����������Ă���
	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));

	//��������肭�������N���ł��Ȃ�
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	//RTV�̐ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//�o�͌��ʂ�SRGB�ɕϊ����ď�������
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2d�e�N�X�`���Ƃ��ď�������

	//�f�B�X�N���v�^�̐擪�����Ď擾
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//2�̃f�B�X�N���v�^��p��
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	rtvHandles[0] = rtvStartHandle;

	//�P�ڂ��쐬(�ŏ��͍��ꏊ���w�肷��K�v������)
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);

	//�Q�ڂ��쐬(���͂ō쐬����)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);



	//�������ރo�b�N�o�b�t�@�̃C���f�b�N�X���擾
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//TransitionBarrier�̐ݒ�
	D3D12_RESOURCE_BARRIER barrier{};
	//����̃o���A��Transition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//None�ɂ��Ă���
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//�o���A�𒣂�Ώۂ̃��\�[�X�B���݂̃o�b�N�o�b�t�@�ɑ΂��čs��
	barrier.Transition.pResource = swapChainResources[backBufferIndex];
	//�Ҍ��O(����)��ResourceStart
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//�Ҍ���ResourceStart
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//TransitionBarrier�𒣂�
	commandList->ResourceBarrier(1, &barrier);


	//�`����RTV��ݒ肷��
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	//�w�肵���F�ŉ�ʑS�̂��N���A����
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	//��ʂɕ`�������͑S�ďI���A��ʂɈڂ��̂ŁA��Ԃ��Ҍ�
	//�����RenderTarget����Present�ɂ���
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrier�𒣂�
	commandList->ResourceBarrier(1, &barrier);

	//�R�}���h���X�̓��e���m��B�S�Đς�ł���Close���邱��
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	//GPU�ɃR�}���h���X�g�̎��s���s�킹��
	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists);

	//GPU��OS�ɉ�ʂ̌������s���悤�ɒʒm����
	swapChain->Present(1, 0);


	//�����l��0��fence�����
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//Fence��Signal��҂��߂̃C�x���g���쐬����
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);


	//dxcCompiler��������
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxCompiler));
	assert(SUCCEEDED(hr));

	//include�ɑΉ����邽�߂̐ݒ�
	IDxcIncludeHandler* IincludeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&IincludeHandler);
	assert(SUCCEEDED(hr));

	//fence�̒l���X�V
	fenceValue++;
	//GPU�������܂ł��ǂ蒅�������ɁBFence�̒l���w�肵���l�ɑ������悤��Signal�𑗂�
	commandQueue->Signal(fence, fenceValue);

	//Fence�̒l���w�肵��Singnal�l�ɂ��ǂ蒅���Ă��邩�m�F����
	//GetCompletedValue�̏����l��Fence�쐬���ɓn���������l
	if (fence->GetCompletedValue() < fenceValue) {
		//�w�肵��sighal�ɂ��ǂ���Ă��Ȃ��̂ŁA���ǂ蒅���܂ő҂悤�ɃC�x���g��ݒ肷��
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//�C�x���g��҂�
		WaitForSingleObject(fenceEvent, INFINITE);

	}


	//RootSignature�쐬
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//�V���A���C�Y���ăo�C�i���ɂ���
	ID3DBlob* signatureBlod = nullptr;
	ID3DBlob* errorBlod = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlod, &errorBlod);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlod->GetBufferPointer()));
		assert(false);
	}

	//�o�C�i�������ɐ���
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlod->GetBufferPointer(), signatureBlod->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));


	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputELementDescs[1] = {};
	inputELementDescs[0].SemanticName = "POSITION";
	inputELementDescs[0].SemanticIndex = 0;
	inputELementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputELementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputELementDescs;
	inputLayoutDesc.NumElements = _countof(inputELementDescs);

	// BlendState�̐ݒ�
	D3D12_BLEND_DESC blendDesc{};
	//�S�Ă̐F�v�f����������
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//ResitezerState�̐ݒ�
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//����(���v����)��\�����Ȃ�
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//shader���R���p�C������
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3D.VS.hlsl", L"vs_6_0",dxcUtils,dxCompiler,IincludeHandler);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3D.PS.hlsl", L"vs_6_0", dxcUtils, dxCompiler, IincludeHandler);
	assert(pixelShaderBlob != nullptr);




	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicspipelineStateDesc{};
	graphicspipelineStateDesc.pRootSignature = rootSignature;
	graphicspipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicspipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize()};
	graphicspipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };
	graphicspipelineStateDesc.BlendState = blendDesc;
	graphicspipelineStateDesc.RasterizerState = rasterizerDesc;
	//��������RTV�̏��
	graphicspipelineStateDesc.NumRenderTargets = 1;
	graphicspipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//���p����ƃ|�����W(�`��)�̃^�C�v
	graphicspipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//�ǂ̂悤�ɉ�ʂɐF��ł����ނ��̐ݒ�
	graphicspipelineStateDesc.SampleDesc.Count = 1;
	graphicspipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//���ۂɐ���
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicspipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));




	//���_���\�[�X�p�̃q�[�v�̐ݒ�
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	//���_���\�[�X�̐ݒ�
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//�o�b�t�@���\�[�X�A�e�N�X�`���̏ꍇ�͂܂��ʂ̐ݒ������
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(Vector4) * 3;

	//�o�b�t�@�̏ꍇ�͂����͂P�ɂ���
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//�o�b�t�@�̏ꍇ�͂���ɂ���
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//���ۂɒ��_���\�[�X�����
	ID3D12Resource* vertexResource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties,D3D12_HEAP_FLAG_NONE,&vertexResourceDesc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));



	//���_�o�b�t�@�r���[���쐬
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//���\�[�X�̐擪�̃A�h���X����g��
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//�g�p���郊�\�[�X�̃T�C�Y�͒��_�R���̃T�C�Y
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	//�P���_������̃T�C�Y
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	//���_���\�[�X�Ƀf�[�^����������
	Vector4* vertexDate = nullptr;
	//�������ނ��߂̃A�h���X���擾
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDate));
	//����
	vertexDate[0] = {-0.5f,-0.5f,0.0f,1.0f};
	//��
	vertexDate[1] = {0.0f,0.5f,0.0f,1.0f};
	//�E��
	vertexDate[2] = {0.5f,-0.5f,0.0f,1.0f};


	//�r���[�|�[�g
	D3D12_VIEWPORT viewport{};
	//�N���C�A���g�̈�̃T�C�Y�ƈꏏ�ɂ��ĉ�ʑS�̂ɕ\��
	viewport.Width = kClientWidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;


	//�V�U�[��`
	D3D12_RECT scissorRect{};
	//��{�I�Ƀr���[�|�[�g�Ɠ�����`���\�������
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;



	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	//RootSignature��ݒ�
	commandList->SetGraphicsRootDescriptorTable(rootSignature);
	commandList->SetPipelineState(graphicsPipelineState);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	//�`���ݒ�
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//	�`��
	commandList->DrawInstanced(3, 1, 0, 0);



	//���̃t���[���p�̃R�}���h���X�g������
	hr = commandList->Reset(commandAllocator, nullptr);
	assert(SUCCEEDED(hr));

	MSG msg{};
	//�E�B���h�E�̂��{�^�����������܂Ń��[�v
	while (msg.message != WM_QUIT) {
		//Window�Ƀ��b�Z�[�W��������ŗD��ŏ���������
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//�Q�[���̏���

		}
	}

	CloseHandle(fenceEvent);
	fence->Release();
	rtvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();


	vertexResource->Release();
	graphicsPipelineState->Release();
	if (errorBlod) {
		errorBlod->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();

#ifdef _DEBUG
	debugController->Release();
#endif 
	CloseWindow(hwnd);


	//���\�[�X���[�N�`�F�b�N
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	//�x�����Ɏ~�܂�	
	//InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING,true);

	return 0;
}
