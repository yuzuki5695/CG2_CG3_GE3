#pragma once
#include<Windows.h>
#include<wrl.h>
#define DIRECTINPUT_VERSION   0x0800 // DirectInputのバージョン指定
#include<dinput.h>

// 入力
class Input
{
public: // メンバ関数

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 初期化
	void Initialize(HINSTANCE hInstance,HWND hwnd);
	// 更新
	void Update();
		
	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber"キー番号( DIK_0 等)</param>
	bool Pushkey(BYTE keyNumber);

	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber"キー番号( DIK_0 等)</param>
	/// <returns>トリガーか</returns>
	bool Triggrkey(BYTE keyNumber);

private: // メンバ変数
	//キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;
	// 全キーの状態
	BYTE key[256] = {};
	// 前回の全キーの状態
	BYTE keyPre[256] = {};
	// DirectInputのインスタンス生成
	ComPtr<IDirectInput8> directInput = nullptr;
};

