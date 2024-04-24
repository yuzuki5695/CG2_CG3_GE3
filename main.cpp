#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include<dxgidebug.h>

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
