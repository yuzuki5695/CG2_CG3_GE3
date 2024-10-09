#include "WinApp.h"

void WinApp::Initialize() {
 

    WNDCLASS wc{};
    //ウィンドウプロシージャ
    wc.lpfnWndProc = WindowProc;
    //ウィンドウクラス名
    wc.lpszClassName = L"CG2WindowClass";
    //インスタンスハンドル
    wc.hInstance = GetModuleHandle(nullptr);
    //カーソル
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //ウィンドウクラスを登録する
    RegisterClass(&wc);

    //クライアント領域のサイズ
    const int32_t kClientWidth = 1280;
    const int32_t kClientHeight = 720;
    //ウィンドウサイズを表す構造体にクライアント領域を入れる
    RECT wrc = { 0,0,kClientWidth,kClientHeight };
    //クライアント領域を元に実際のサイズにwrcを変更してもらう
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    //ウィンドウの生成
    HWND hwnd = CreateWindow(
        wc.lpszClassName,        //利用するクラス名
        L"CG2",                  //タイトルバーの文字
        WS_OVERLAPPEDWINDOW,     //よく見るウィンドウスタイル
        CW_USEDEFAULT,           //表示X座標(Windowsに任せる)
        CW_USEDEFAULT,           //表示Y座標(WindowsOSに任せる)
        wrc.right - wrc.left,    //ウィンドウ横幅
        wrc.bottom - wrc.top,    //ウィンドウ縦幅
        nullptr,                 //親ウィンドウハンドル
        nullptr,                 //メニューハンドル
        wc.hInstance,            //インスタンスハンドル
        nullptr);                //オプション 

    //ウィンドウを表示する
    ShowWindow(hwnd, SW_SHOW);


}


void WinApp::Update() {


}

//ウィンドウプロージャー
LRESULT CALLBACK WinApp::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    //メッセージ二応じてゲーム固有の処理を行う

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return true;
    }

    switch (msg) {
        //ウィンドウが破棄された
    case WM_DESTROY:
        //OSに対して、アプリの終了を伝える
        PostQuitMessage(0);
        return 0;
    }
    //標準のメッセージ処理を行う
    return DefWindowProc(hWnd, msg, wParam, lParam);
}