#include "Input.h"
#include<cassert>
#include<wrl.h>
#define DIRECTINPUT_VERSION   0x0800 // DirectInputのバージョン指定
#include<dinput.h>
#pragma comment(lib,"dinput8.lid")
#pragma comment(lib,"dxguid.lid")

using namespace Microsoft::WRL;

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	HRESULT result;

	// DirectInputのインスタンス生成
	ComPtr<IDirectInput8> directInput = nullptr;
	result = DirectInput8Create(hInstance,DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&directInput,nullptr);
	assert(SUCCEEDED(result));
	// キーボ－ドデバイス生成
	ComPtr<IDirectInputDevice8> keyboard;
	result = directInput->CreateDevice(GUID_SysKeyboard,&keyboard,NULL);
	assert(SUCCEEDED(result));
	// 入力データの形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	// 排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}


void Input::Update() 
{

}