#pragma once
#include<Windows.h>
#include<cstdint>
#include<minwindef.h>
#include"externals/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class WinApp
{
public: // 静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);

public: // メンバ関数

	// 初期化
	void Initialize();
	// 更新
	void Update();

	// getter
	HWND Gethwnd() const { return hwnd; }
	HINSTANCE GetHInstance() const { return wc.hInstance; }

public: // 定数
	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

private:
	// ウィンドウハンドル
	HWND hwnd = nullptr;
	// ウィンドウクラスの設定
	WNDCLASS wc{};
};

