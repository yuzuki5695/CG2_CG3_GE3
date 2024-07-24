#include<Windows.h>
#include<cstdint>
#include<string>
#include<format>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#include<dxgidebug.h>
#include<dxcapi.h>
#include<cmath>
#include<assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
#include"MatrixVector.h"
#include"Resource.h"
#include<fstream>
#include<sstream>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")


struct Transform {
    Vector3 scale;
    Vector3 rotate;
    Vector3 translate;
};

struct VertexData
{
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};

struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
};

struct Material {
    Vector4 color;
    int32_t endbleLighting;
    float padding[3];
    Matrix4x4 uvTransform;
};

struct DirectionalLight {
    Vector4 color; //!< ライトの色
    Vector3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};

struct MaterialDate {
    std::string textureFilePath;
};


struct ModelDate {
    std::vector<VertexData> vertices;
    MaterialDate material;
};

//ウィンドウプロージャー
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
void Log(const std::string& message) {
    OutputDebugStringA(message.c_str());;
}

std::wstring ConvertString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }

    auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
    if (sizeNeeded == 0) {
        return std::wstring();
    }
    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
    return result;
}

std::string ConvertString(const std::wstring& str) {
    if (str.empty()) {
        return std::string();
    }

    auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
    if (sizeNeeded == 0) {
        return std::string();
    }
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
    return result;
}

// コンパイルシェーダー
Microsoft::WRL::ComPtr <IDxcBlob> CompileShader(
    //CompileするShaderファイルのパス
    const std::wstring& filePath,
    //Compilerに使用するProfile
    const wchar_t* profile,
    //初期化で生成したものを3つ
    Microsoft::WRL::ComPtr <IDxcUtils> dxcUtils,
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler,
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler) {


    //1.hlslファイルを読む
    //これからシェーダーをコンパイルする旨をログに出す
    Log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
    Microsoft::WRL::ComPtr <IDxcBlobEncoding> shaderSource = nullptr;
    HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
    //読めなかったら止める
    assert(SUCCEEDED(hr));

    //読み込んだファイルの内容を設定する
    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8のコードであることを通知

    //2.Compileする
    LPCWSTR arguments[] =
    {

         filePath.c_str(),
         L"-E",L"main",
         L"-T",profile,
         L"-Zi",L"-Qembed_debug",
         L"-Od",
         L"-Zpr",
    };
    //実際にshaderをコンパイルする
    Microsoft::WRL::ComPtr <IDxcResult> shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSourceBuffer,
        arguments,
        _countof(arguments),
        includeHandler.Get(),
        IID_PPV_ARGS(&shaderResult)
    );

    assert(SUCCEEDED(hr));

    //警告・エラーが出てたらログを出して止める
    Microsoft::WRL::ComPtr <IDxcBlobUtf8> shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
        Log(shaderError->GetStringPointer());
        assert(false);
    }

    //コンパイル結果から実行用のバイナリ部分を取得
    Microsoft::WRL::ComPtr <IDxcBlob> shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));
    //成功したログを出す
    Log(ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
    //もう使わないリソースを解放
    shaderSource->Release();
    shaderResult->Release();
    //実行用のバイナリを返却
    return shaderBlob;

}



Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(
    Microsoft::WRL::ComPtr <ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
    //ディスクリプタヒープの生成
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = heapType;
    descriptorHeapDesc.NumDescriptors = numDescriptors;
    descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
    //ディスクリプタヒープが作れなかったので起動できない
    assert(SUCCEEDED(hr));
    return descriptorHeap;
}

//Textureデータを読む
DirectX::ScratchImage LoadTexture(const std::string& filePath)
{
    //テクスチャファイルを呼んでプログラムで扱えるようにする
    DirectX::ScratchImage image{};
    std::wstring filePathW = ConvertString(filePath);
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));

    //ミップマップの作成
    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
    assert(SUCCEEDED(hr));

    //ミップマップ付きのデータを返す
    return mipImages;
}

//TextureResourceにデータを移送する
void UploadTextureData(Microsoft::WRL::ComPtr <ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages)
{
    //Meta情報を取得
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    //全MipMapについて
    for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel)
    {
        //MipMapLevelを指定して各Imageを取得
        const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
        //Textureに転送
        HRESULT hr = texture->WriteToSubresource(
            UINT(mipLevel),
            nullptr,				//全領域へコピー
            img->pixels,			//元データアドレス
            UINT(img->rowPitch),	//1ラインサイズ
            UINT(img->slicePitch)	//1枚サイズ
        );
        assert(SUCCEEDED(hr));
    }
}


bool DepthFunc(float currZ, float prevZ) {
    return currZ <= prevZ;
}

//bool DepthFunc(float currZ, float prevZ) {
//    return currZ >= prevZ;
//}


/*------------------------------------------------------------------------------------*/
/*-------------------------------------球の作成関数-------------------------------------*/
/*------------------------------------------------------------------------------------*/

void DrawSphere(const uint32_t ksubdivision, VertexData* vertexdata) {
    // 球の頂点数を計算する
    //経度分割1つ分の角度 
    const float kLonEvery = (float)M_PI * 2.0f / float(ksubdivision);
    //緯度分割1つ分の角度 
    const float kLatEvery = (float)M_PI / float(ksubdivision);
    //経度の方向に分割
    for (uint32_t latIndex = 0; latIndex < ksubdivision; ++latIndex)
    {
        float lat = -(float)M_PI / 2.0f + kLatEvery * latIndex;	// θ
        //経度の方向に分割しながら線を描く
        for (uint32_t lonIndex = 0; lonIndex < ksubdivision; ++lonIndex)
        {
            float u = float(lonIndex) / float(ksubdivision);
            float v = 1.0f - float(latIndex) / float(ksubdivision);

            //頂点位置を計算する
            uint32_t start = (latIndex * ksubdivision + lonIndex) * 6;
            float lon = lonIndex * kLonEvery;	// Φ
            //頂点にデータを入力する。基準点 a
            vertexdata[start + 0].position = { cos(lat) * cos(lon) ,sin(lat) , cos(lat) * sin(lon) ,1.0f };
            vertexdata[start + 0].texcoord = { u,v };
            vertexdata[start + 0].normal.x = vertexdata[start + 0].position.x;
            vertexdata[start + 0].normal.y = vertexdata[start + 0].position.y;
            vertexdata[start + 0].normal.z = vertexdata[start + 0].position.z;

            //基準点 b
            vertexdata[start + 1].position = { cos(lat + kLatEvery) * cos(lon),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon) ,1.0f };
            vertexdata[start + 1].texcoord = { u ,v - 1.0f / float(ksubdivision) };
            vertexdata[start + 1].normal.x = vertexdata[start + 1].position.x;
            vertexdata[start + 1].normal.y = vertexdata[start + 1].position.y;
            vertexdata[start + 1].normal.z = vertexdata[start + 1].position.z;

            //基準点 c
            vertexdata[start + 2].position = { cos(lat) * cos(lon + kLonEvery),sin(lat), cos(lat) * sin(lon + kLonEvery) ,1.0f };
            vertexdata[start + 2].texcoord = { u + 1.0f / float(ksubdivision),v };
            vertexdata[start + 2].normal.x = vertexdata[start + 2].position.x;
            vertexdata[start + 2].normal.y = vertexdata[start + 2].position.y;
            vertexdata[start + 2].normal.z = vertexdata[start + 2].position.z;

            //基準点 d
            vertexdata[start + 3].position = { cos(lat + kLatEvery) * cos(lon + kLonEvery), sin(lat + kLatEvery) , cos(lat + kLatEvery) * sin(lon + kLonEvery) ,1.0f };
            vertexdata[start + 3].texcoord = { u + 1.0f / float(ksubdivision), v - 1.0f / float(ksubdivision) };
            vertexdata[start + 3].normal.x = vertexdata[start + 3].position.x;
            vertexdata[start + 3].normal.y = vertexdata[start + 3].position.y;
            vertexdata[start + 3].normal.z = vertexdata[start + 3].position.z;

            // 頂点4 (b, c, d)
            vertexdata[start + 4].position = { cos(lat) * cos(lon + kLonEvery),sin(lat),cos(lat) * sin(lon + kLonEvery),1.0f };
            vertexdata[start + 4].texcoord = { u + 1.0f / float(ksubdivision) ,v };
            vertexdata[start + 4].normal.x = vertexdata[start + 4].position.x;
            vertexdata[start + 4].normal.y = vertexdata[start + 4].position.y;
            vertexdata[start + 4].normal.z = vertexdata[start + 4].position.z;

            vertexdata[start + 5].position = { cos(lat + kLatEvery) * cos(lon),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon),1.0f };
            vertexdata[start + 5].texcoord = { u,v - 1.0f / float(ksubdivision) };
            vertexdata[start + 5].normal.x = vertexdata[start + 5].position.x;
            vertexdata[start + 5].normal.y = vertexdata[start + 5].position.y;
            vertexdata[start + 5].normal.z = vertexdata[start + 5].position.z;
        }
    }
}


/*-----------------------------------------------------------------------------------*/
/*-------------------------CPUのディスクリプタハンドルの取得関数---------------------------*/
/*-----------------------------------------------------------------------------------*/

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index)
{

    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorsize * index);
    return handleCPU;
}

/*-----------------------------------------------------------------------------------*/
/*-------------------------GPUのディスクリプタハンドルの取得関数---------------------------*/
/*-----------------------------------------------------------------------------------*/

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorsize, uint32_t index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorsize * index);
    return handleGPU;
}

/*----------------------------------------------------------------------*/
/*-------------------------Objファイルを読む関数---------------------------*/
/*----------------------------------------------------------------------*/


MaterialDate LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    MaterialDate materialDate; // 構築するMaterialDate
    std::string line; // ファイルから読んだ1行を格納するもの
    std::ifstream file(directoryPath + "/" + filename); // 2.ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める
    // 3. 実際にファイルを読み、MaterialDateを構築していく
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        // identifierの応じた処理
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            // 連結してファイルパスにする
            materialDate.textureFilePath = directoryPath + "/" + textureFilename;
        }
    }
    return materialDate;
}

ModelDate LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    ModelDate modelDate; // 構築するModelDate
    std::vector<Vector4> positions; // 位置
    std::vector<Vector3> normals; // 法線
    std::vector<Vector2> texcoords; // テクスチャ座標
    std::string line; // ファイルから読んだ1桁を格納するもの
    // 2.  ファイルを開く
    std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める

    // 3. 実際にファイルを読み、ModelDateを構築していく
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;// 先頭の識別子を読む

        // identifierの応じた処理
        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.x *= -1.0f;// 位置のx成分を反転
            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoord.y = 1.0f - texcoord.y;
            texcoords.push_back(texcoord);
        } else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.x *= -1.0f;// 法線のx成分を反転
            normals.push_back(normal);
        } else if (identifier == "f") {
            VertexData triangle[3];
            // 面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                // 頂点の要素へのIndexは、[位置/UV/法線]で格納されているので、分解してIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (uint32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/');// /区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }
                // 要素のIndexから、実際の要素の値を取得して、頂点を構築する
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];
                //VertexData vertex = { position,texcoord,normal };
                //modelDate.vertices.push_back(vertex);
                triangle[faceVertex] = { position,texcoord,normal };
            }
            // 頂点を逆順で登録することで、回り順を逆にする
            modelDate.vertices.push_back(triangle[2]);
            modelDate.vertices.push_back(triangle[1]);
            modelDate.vertices.push_back(triangle[0]);
        } else if (identifier == "mtllib") {
            // materialTemplateLibrarvファイルの名前を取得する
            std::string materialFilename;
            s >> materialFilename;
            // 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
            modelDate.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }
    // 4. ModelDateを返す
    return modelDate;
}

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    OutputDebugStringA("Hello,Directx!\n");

    //COMの初期化
    CoInitializeEx(0, COINIT_MULTITHREADED);

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

    //デバックレイヤー
#ifdef _DEBUG
    Microsoft::WRL::ComPtr <ID3D12Debug1> debugController = nullptr;

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        //デバッグレイヤーを有効化
        debugController->EnableDebugLayer();
        //さらにGPU側でもチェックを行えるようにする
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif // _DEBUG

    /*D3D12Device生成*/
    Microsoft::WRL::ComPtr <ID3D12Device> device = nullptr;
    //IDXGIのファクトリー生成
    Microsoft::WRL::ComPtr <IDXGIFactory7> dxgiFactory = nullptr;
    //HRESULTはWindows系のエラーコードであり、
  //関数が成功したかどうかをSUCCEEDEDマクロで判定できる
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    //初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が
  //多いのでassertにする
    assert(SUCCEEDED(hr));

    //仕様するアダプター用の変数。最初にnullptrを入れておく
    Microsoft::WRL::ComPtr <IDXGIAdapter4> useAdapter = nullptr;
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
            Log(ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
            break;
        }
        useAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
    }
    //適切なアダプタが見つからないので起動できない
    assert(useAdapter != nullptr);
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
            Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
            break;
        }
    }
    //デバイスの生成がうまくいかなかったので起動できない
    assert(device != nullptr);
    Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

#ifdef _DEBUG
    Microsoft::WRL::ComPtr <ID3D12InfoQueue> infoQueue = nullptr;
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

    //コマンドキューを生成する
    Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
    //コマンドキューの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    //コマンドアロケーターを生成する
    Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator = nullptr;
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    //コマンドアロケータの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    //コマンドリストを生成する
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
        IID_PPV_ARGS(&commandList));
    //コマンドリストの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    //SwapChain(スワップチェーン)を生成する
    Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = kClientWidth;//画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Height = kClientHeight;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
    swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2;//ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに移したら、中身居を破棄
    //コマンドキュー、ウィンドウハンドル、設定を渡して生成する
    hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
    assert(SUCCEEDED(hr));

    //DescriptorRange作成
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[4] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[0].Descriptor.ShaderRegister = 0;// レジスタ番号0を使う

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;// VertexShaderでを使う
    rootParameters[1].Descriptor.ShaderRegister = 0;// レジスタ番号0を使う

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//利用する数

    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// CBVを使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;// PixelShaderで使う
    rootParameters[3].Descriptor.ShaderRegister = 1;// レジスタ番号1を使う

    //RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.pParameters = rootParameters;// ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = _countof(rootParameters);// 配列の長さ

    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ	
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0～1の範囲外をリピート		
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない		
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う	
    staticSamplers[0].ShaderRegister = 0;//レジスタ番号０を使う
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


    /*------------------------------------------------------------------------------------*/
    /*----------------------------------Resourceの作成-------------------------------------*/
    /*------------------------------------------------------------------------------------*/

    /*------------------------------------------------------------------*/
    /*----------------------マテリアル用のResource------------------------*/
    /*------------------------------------------------------------------*/

    // マテリアル用のリソース
    Microsoft::WRL::ComPtr <ID3D12Resource> materialResource = CreateBufferResource(device, sizeof(Material));
    // マテリアル用にデータを書き込む
    Material* materialData = nullptr;
    // 書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // 今回は白
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    // Lightingするのでtrueを設定する
    materialData->endbleLighting = true;
    // 単位行列を書き込んでおく
    materialData->uvTransform = MakeIdentity4x4();

    /*------------------------------------------------------------------*/
    /*----------------TransformationMatrix用のResource-------------------*/
    /*------------------------------------------------------------------*/

     // WVP,World用のリソースを作る。TransformationMatrixを用意する
    Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
    // データを書き込む
    TransformationMatrix* transformationMatrixData = nullptr;
    // 書き込むためのアドレスを取得
    wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
    // 単位行列を書き込んでおく
    transformationMatrixData->WVP = MakeIdentity4x4();
    transformationMatrixData->World = MakeIdentity4x4();

    /*------------------------------------------------------------------*/
    /*------------------------Sprite用のResource-------------------------*/
    /*------------------------------------------------------------------*/

     //Sprite用のマテリアルリソースを作る
    Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceSprite = CreateBufferResource(device, sizeof(Material));
    // Sprite用にデータを書き込む
    Material* materialSpriteDate = nullptr;
    // 書き込むためのアドレスを取得
    materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialSpriteDate));
    // 今回は白
    materialSpriteDate->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    // SpriteはLightingしないでfalseを設定する
    materialSpriteDate->endbleLighting = false;
    // 単位行列を書き込んでおく
    materialSpriteDate->uvTransform = MakeIdentity4x4();

    /*------------------------------------------------------------------*/
    /*-----------------------平行光源用のResource-------------------------*/
    /*------------------------------------------------------------------*/

     // 平行光源用のリソースを作る
    Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
    // 平行光源用にデータを書き込む
    DirectionalLight* directionalLightDate = nullptr;
    // 書き込むためのアドレスを取得
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDate));
    // デフォルト値はとりあえず以下のようにして置く
    directionalLightDate->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightDate->direction = { 0.0f,-1.0f,0.0f };
    directionalLightDate->intensity = 1.0f;

    /*-------------------------------------------------------*/
   /*----------------------球のデータ-------------------------*/
   /*-------------------------------------------------------*/

    const uint32_t kSubdivision = 16; //球の分割数

    uint32_t vertexCount = kSubdivision * kSubdivision * 6; //球の頂点数

    // モデル読み込み
    ModelDate modelDate = LoadObjFile("resources", "axis.obj");

    // 関数化したResouceで作成
    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce = CreateBufferResource(device, sizeof(VertexData) * modelDate.vertices.size());

    /*-----------------------------------------------------*/
    /*-----------------------球の描画-----------------------*/
    /*-----------------------------------------------------*/

    //頂点バッファビューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    // リソースの先頭のアドレスから使う
    vertexBufferView.BufferLocation = vertexResoruce->GetGPUVirtualAddress();
    // 使用するリソースのサイズはの頂点のサイズ
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelDate.vertices.size());
    // 1頂点当たりのサイズ
    vertexBufferView.StrideInBytes = sizeof(VertexData);

    //頂点リソースにデータを書き込む
    VertexData* vertexData = nullptr;
    //書き込むためのアドレスを取得
    vertexResoruce->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    // 頂点データをリソースにコピー
    std::memcpy(vertexData, modelDate.vertices.data(), sizeof(VertexData) * modelDate.vertices.size());

    // 球の頂点にデータを入力
   // DrawSphere(kSubdivision, vertexData);

    /*-------------------------------------------------------*/
    /*----------------------spriteのデータ---------------------*/
    /*------------------------------------------------------*/

    // Sprite用の頂点リソースを作る
    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruceSprite = CreateBufferResource(device, sizeof(VertexData) * 4);

    //頂点バッファビューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
    // リソースの先頭のアドレスから使う
    vertexBufferViewSprite.BufferLocation = vertexResoruceSprite->GetGPUVirtualAddress();
    // 使用するリソースのサイズは4つ分のサイズ
    vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
    // 1頂点当たりのサイズ
    vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

    //頂点リソースにデータを書き込む
    VertexData* vertexDataSprite = nullptr;
    //書き込むためのアドレスを取得
    vertexResoruceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

    vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
    vertexDataSprite[0].texcoord = { 0.0f,1.0f };
    vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
    vertexDataSprite[1].texcoord = { 0.0f,0.0f };
    vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
    vertexDataSprite[2].texcoord = { 1.0f,1.0f };
    vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };
    vertexDataSprite[3].texcoord = { 1.0f,0.0f };
    for (int i = 0; i < 4; i++) {
        vertexDataSprite[i].normal = { 0.0f,0.0f,-1.0f };
    }

    Microsoft::WRL::ComPtr <ID3D12Resource> indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);
    //頂点バッファビューを作成する
    D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
    // リソースの先頭のアドレスから使う
    indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
    // 使用するリソースのサイズは6つ分のサイズ
    indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
    // インデックスはuint32_tとする
    indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
    // インデックスリソースにデータを書き込む
    uint32_t* indexDateSprite = nullptr;
    indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDateSprite));
    indexDateSprite[0] = 0; indexDateSprite[1] = 1; indexDateSprite[2] = 2;
    indexDateSprite[3] = 1; indexDateSprite[4] = 3; indexDateSprite[5] = 2;

    // Sprite用のTransformationMatrix用のリソースを作る。
    Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
    // データを書き込む
    TransformationMatrix* transformationMatrixDateSprite = nullptr;
    // 書き込むためのアドレスを取得
    transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDateSprite));
    // 単位行列を書き込んでおく
    transformationMatrixDateSprite->World = MakeIdentity4x4();
    transformationMatrixDateSprite->WVP = MakeIdentity4x4();

    /*-----------------------------------------------------------------------------------*/
    /*--------------------------------Resourceの作成終了-----------------------------------*/
    /*-----------------------------------------------------------------------------------*/

     //シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    //バイナリを元に作成
    Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    //======== InputLayout設定 ==========//
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    //======= BlendStateの設定 =========//
    D3D12_BLEND_DESC blendDesc{};
    //全ての色要素を書き込む
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    //===== RasterizerStateの設定を行う ======//   
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    //裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    //三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    /*----------------------------------------------------------------------------*/
    /*---------------------------DescriptorHeap-----------------------------------*/
    /*----------------------------------------------------------------------------*/

    // RTV用のヒープでディスクリプタの数は2。RTVはshader内で触るものではないので、ShaderVisibleはfalse
    Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
    // SRV用のヒープでディスクリプタの数は128.RTVはshader内で触るものなので、ShaderVisibleはtrue
    Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
    // DSV用のヒープでディスクリプタの数は1。DSVはshader内で触るものではないので、ShaderVisibleはfalse
    Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

    // DescriptorSizeを取得する
    const uint32_t descriptorsizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const uint32_t descriptorsizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const uint32_t descriptorsizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    //SwapChainからResourceを引っ張ってくる
    Microsoft::WRL::ComPtr <ID3D12Resource> swapChainResources[2] = { nullptr };
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
    //上手く取得できなければ起動できない
    assert(SUCCEEDED(hr));
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
    assert(SUCCEEDED(hr));


    //初期値0でFenceを作る
    Microsoft::WRL::ComPtr <ID3D12Fence> fence = nullptr;
    uint64_t fenceValue = 0;
    hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));
    //FenceのSignalを待つためのイベントを作成する
    HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);

    // dxCompilerを初期化
    Microsoft::WRL::ComPtr <IDxcUtils> dxcUtils = nullptr;
    Microsoft::WRL::ComPtr <IDxcCompiler3> dxcCompiler = nullptr;
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));

    /*-------------------------------------------------------------------------------*/
    /*----------------------------各Descriptorの設定----------------------------------*/
    /*-------------------------------------------------------------------------------*/


     /*-----------------------------------------------------------*/
    /*--------------------------RTVの設定--------------------------*/
    /*------------------------------------------------------------*/

     //RTVの設定
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGB2変換して書き込む
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャとして読み込む
    //ディスクリプタの先頭を取得する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorsizeRTV, 0);
    //RTVを2つ作るのでディスクリプタを2つ用意
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
    //まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
    rtvHandles[0] = rtvStartHandle;
    device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
    //2つ目のディスクリプタハンドルを得る
    rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    //2つ目を作る
    device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

    //現時点でincludeはしないが、includeに対応するための設定を行っていく
    Microsoft::WRL::ComPtr <IDxcIncludeHandler> includeHandler = nullptr;
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));

    /*------------------------------------------------------------*/
    /*--------------------------SRVの設定--------------------------*/
    /*------------------------------------------------------------*/

    //Textureを読んで転送する
    DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    Microsoft::WRL::ComPtr <ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
    UploadTextureData(textureResource, mipImages);

    //2枚目のTextureを読んで転送する
    DirectX::ScratchImage mipImages2 = LoadTexture(modelDate.material.textureFilePath);
    const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
    Microsoft::WRL::ComPtr <ID3D12Resource> textureResource2 = CreateTextureResource(device, metadata2);
    UploadTextureData(textureResource2, mipImages2);

    //metaDataを基にSRVの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;				//2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
    srvDesc2.Format = metadata2.format;
    srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;				//2Dテクスチャ
    srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

    //SRVを作成するDescriptorHeapの場所を決める
    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorsizeSRV, 0);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorsizeSRV, 0);
    //先頭はImGuiが使っているのでその次を使う
    textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    //SRVの生成
    device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

    //SRVを作成するDescriptorHeapの場所を決める
    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorsizeSRV, 1);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorsizeSRV, 1);
    //先頭はImGuiが使っているのでその次を使う
    textureSrvHandleCPU2.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    textureSrvHandleGPU2.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    //SRVの生成
    device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

    /*------------------------------------------------------------*/
    /*--------------------------DSVの設定--------------------------*/
    /*------------------------------------------------------------*/

    // DepthStencilTextureをウインドウのサイズで作成
    Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeight);

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

    /*----------------------------------------------------------------------------------*/
    /*----------------------------各Descriptorの設定終了----------------------------------*/
    /*----------------------------------------------------------------------------------*/

     //======== ShaderをCompile ===========// 
    Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = CompileShader(L"Object3D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob = CompileShader(L"Object3D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(pixelShaderBlob != nullptr);

    //========== PSO生成 =============//
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();   // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;    // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };  // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };    // PixelShader
    graphicsPipelineStateDesc.BlendState = blendDesc;// BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; //RasterizerState
    // 書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    //利用するトポロジ(形状)のタイプ
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // どのように画面に色を打ち込むかの設定(気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 実際に生成
    Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
    hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
    assert(SUCCEEDED(hr));


    // ビューポート
    D3D12_VIEWPORT viewport{};
    //クライアント領域のサイズと一緒にして画面全体に表示
    viewport.Width = kClientWidth;
    viewport.Height = kClientHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // シザー短形
    D3D12_RECT scissorRect{};
    // 基本的にビューポートと同じ矩形が構成されるようにする
    scissorRect.left = 0;
    scissorRect.right = kClientWidth;
    scissorRect.top = 0;
    scissorRect.bottom = kClientHeight;

    Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

    Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

    Transform  cameratransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-500.0f} };

    Transform  uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

    //-----------------------------//
    //-------ImGuiの初期化-----------//
    //-----------------------------//
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(device.Get(),
        swapChainDesc.BufferCount,
        rtvDesc.Format,
        srvDescriptorHeap.Get(),
        srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    bool useMonsterBall = true;

    MSG msg{};
    // ウィンドウの×ボタンが押されるまでループ
    while (msg.message != WM_QUIT) {
        // Windowにメッセージが来ていたら最優先で処理させる
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // ゲームの処理

            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
            ImGui::ShowDemoWindow();

            ImGui::Begin("Sprite");
            ImGui::DragFloat3("translate", &transform.translate.x, 0.01f);
            ImGui::SliderAngle("SphererRotateX", &transform.rotate.x);
            ImGui::SliderAngle("SphererRotateY", &transform.rotate.y);
            ImGui::SliderAngle("SphererRotateZ", &transform.rotate.z);
            ImGui::ColorEdit3("colorSprite", reinterpret_cast<float*>(materialSpriteDate));
            ImGui::Checkbox("useMonsterBall", &useMonsterBall);
            ImGui::DragFloat3("LightDirection", &directionalLightDate->direction.x, 0.01f);
            ImGui::DragFloat("LightIntensity", &directionalLightDate->intensity, 0.01f);
            ImGui::DragFloat3("SpriteTranslate", (&transformSprite.translate.x));
            ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
            ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
            ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
            ImGui::End();

            ImGui::Render();

            /*------------------------------------------*/
            /*---------MVP,WorldMatrixの行列を作る--------*/
            /*------------------------------------------*/

           // transform.rotate.y += 0.01f;

            Matrix4x4 worludMatrix = MakeAftineMatrix(transform.scale, transform.rotate, transform.translate);
            Matrix4x4 cameraMatrix = MakeAftineMatrix(cameratransform.scale, cameratransform.rotate, cameratransform.translate);
            Matrix4x4 viewMatrix = Inverse(cameraMatrix);
            Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
            Matrix4x4 worldViewProjectionMatrix = Multiply(worludMatrix, Multiply(viewMatrix, projectionMatrix));
            transformationMatrixData->World = worludMatrix;
            transformationMatrixData->WVP = worldViewProjectionMatrix;

            /*-------------------------------------------*/
            /*---Sprite用のWrldViewProjectionMatrixを作る---*/
            /*--------------------------------------------*/

            Matrix4x4 worludMatrixSprite = MakeAftineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
            Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
            Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
            Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worludMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
            transformationMatrixDateSprite->World = worludMatrixSprite;
            transformationMatrixDateSprite->WVP = worldViewProjectionMatrixSprite;

            /*----------------------------------------*/
            /*---------UVTransform用の行列を作る--------*/
            /*----------------------------------------*/

            Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
            uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
            uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
            materialSpriteDate->uvTransform = uvTransformMatrix;

            // 描画用のDescriptorHeapの設定
            ID3D12DescriptorHeap* descriptorHeap[] = { srvDescriptorHeap.Get()};
            commandList->SetDescriptorHeaps(1, descriptorHeap);

            // ここから書き込むバックバッファのインデックスを取得
            UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
            // TransitionBarrierの設定
            D3D12_RESOURCE_BARRIER barrier{};
            // 今回のバリアはTransition
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            // Noneにしておく
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            // バリアを張る対象のリソース。現在のバックバッファに対して行う
            barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
            // 遷移前のResourceState
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            // 遷移後のResourceState
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            // TransitionBarrierを張る
            commandList->ResourceBarrier(1, &barrier);

            // 描画先のRTVとDSVを設定する
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            // 描画先のRTVを指定する
            commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
            // 指定した色で画面全体をクリアする
            float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
            commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);
            // 指定した深度で画面全体をクリアする
            commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
            // RootSignatureを設定。PSOに設定しているけど別途設定が必要
            commandList->SetGraphicsRootSignature(rootSignature.Get());
            commandList->SetPipelineState(graphicsPipelineState.Get());
            commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
            // 形状を設定。PSOに設定しているものとはまた別。同じものを設定する
            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            // マテリアルCBufferの場所を設定
            commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
            // wvp用のCBufferの場所を設定
            commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
            //SRVのDescriptortableの先頭を設定。２はrootParameter[2]である。
            //SRVを切り替えて画像を変えるS
            commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
            // 平行光源用のCBufferの場所を設定 
            commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

            // 描画！(今回は球) 
            commandList->DrawInstanced(UINT(modelDate.vertices.size()), 1, 0, 0);

            /*---------------------------------------------------*/
            /*-------------------2dの描画コマンド開始---------------*/
            /*---------------------------------------------------*/

             // Spriteの描画は常にuvCheckerにする
            commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

            commandList->IASetIndexBuffer(&indexBufferViewSprite);//IBVを設定
            // wvp用のCBufferの場所を設定
            commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
            // Spriteの描画。変更が必要なものだけ変更する
            commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
            // TransformationMatrixBufferの場所を設定
            commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
            // 描画! (DrawCall/ドローコール) 6個のインデックスを使用し1つのインスタンスを描画、その他は当面０で良い
           // commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

            /*---------------------------------------------------*/
            /*-------------------2dの描画コマンド終了---------------*/
            /*---------------------------------------------------*/

            //実際のcommandListのImGuiの描画コマンドを積む
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

            // 画面に描く処理はすべて終わり、画面に移すので、状態を遷移
            // 今回はRenderTargetからPresentにする
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            // TransitionBarrierを張る
            commandList->ResourceBarrier(1, &barrier);
            // コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
            hr = commandList->Close();
            assert(SUCCEEDED(hr));

            // GPUにコマンドリストのリストの実行を行わせる
            ID3D12CommandList* commandLists[] = { commandList.Get()};
            commandQueue->ExecuteCommandLists(1, commandLists);
            // GPUとOSに画面の交換を行うように通知する
            swapChain->Present(1, 0);
            // Fenceの値の更新
            fenceValue++;
            // GPUがここまでたどり着いたときに、Fenceの値に代入するようにSignalを送る
            commandQueue->Signal(fence.Get(), fenceValue);
            // Fenceの値が指定したSignal値にたどり着いているか確認する
            // GetCompletedValueの初期値はFence作成時に渡した初期値
            if (fence->GetCompletedValue() < fenceValue)
            {
                // 指定したSignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
                fence->SetEventOnCompletion(fenceValue, fenceEvent);
                //イベントを待つ
                WaitForSingleObject(fenceEvent, INFINITE);
            }
            // 次のフレーム用のコマンドリストを準備
            hr = commandAllocator->Reset();
            assert(SUCCEEDED(hr));
            hr = commandList->Reset(commandAllocator.Get(), nullptr);
            assert(SUCCEEDED(hr));

        }
    }

    ///COMの終了
    CoUninitialize();

    // ImGuiの終了処理。
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CloseHandle(fenceEvent);

    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;

    return 0;
}