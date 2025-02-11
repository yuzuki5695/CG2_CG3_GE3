#include"MatrixVector.h"
#include"ResourceObject.h"
#include "Input.h"
#include "DirectXCommon.h"
#include"D3DResourceLeakChecker.h"
#include"Transform.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
#include "Sprite.h" 
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelCommon.h"
#include "Model.h"
#include"ModelManager.h"
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"

// 再帰関数でスプライトを順に描画
void DrawSpritesRecursively(std::vector<Sprite*>& sprites, size_t index) {
    // ベースケース: 全てのスプライトを描画し終えたら終了
    if (index >= sprites.size()) {
        return;
    }
    // 現在のスプライトを描画
    sprites[index]->Draw();
    // 次のスプライトを描画
    DrawSpritesRecursively(sprites, index + 1);
}

// コールバック関数
void DispResult(int* s, std::vector<Object3d*>& objects) {
    // 出目が奇数なら1つ目のオブジェクト、偶数なら2つ目のオブジェクトを表示
    size_t index = (*s % 2 == 1) ? 0 : 1;
    objects[index]->Draw();
}

// コールバック関数型定義
typedef void (*PFunc)(int*, std::vector<Object3d*>&);

// コールバック関数を呼び出す
void setTimeout(PFunc p, int number, int second, std::vector<Object3d*>& objects) {
    Sleep(second * 1000);
    p(&number, objects);
}


//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    OutputDebugStringA("Hello,Directx!\n");

#pragma region ポインタ
    Input* input = nullptr;
    WinApp* winApp = nullptr;
    DirectXCommon* dxCommon = nullptr;
    SpriteCommon* spriteCommon = nullptr;
    Object3dCommon* object3dCommon = nullptr;;
#pragma endregion ポインタ

    // ウィンドウ作成

    // WindowsAPIの初期化
    winApp = new WinApp();
    winApp->Initialize();


    // DirectXの初期化
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

    // テクスチャマネージャの初期化
    TextureManager::GetInstance()->Initialize(dxCommon);
    // 3Dモデルマネージャの初期化
    ModelManager::GetInstance()->Initialize(dxCommon);

    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
    std::string TexturePath01 = "Resources/uvChecker.png";
    std::string TexturePath02 = "Resources/monsterBall.png";
   
    // .objファイルからモデルを読み込む
    ModelManager::GetInstance()->LoadTexture("plane.obj");
    ModelManager::GetInstance()->LoadTexture("axis.obj");
    std::string ModelPath01 = "plane.obj";
    std::string ModelPath02 = "axis.obj";


    // 汎用機能の初期化 

    // 入力の初期化
    input = new Input();
    input->Initialize(winApp);

#pragma region 基盤システムの初期化

    // スプライト共通部の初期化
    spriteCommon = new SpriteCommon;
    spriteCommon->Initialize(dxCommon);

    // 3Dオブジェクト共通部の初期化
    object3dCommon = new Object3dCommon;
    object3dCommon->Initialize(dxCommon);

#pragma endregion 基盤システムの初期化

#pragma region 最初のシーンの初期化

    // スプライトの初期化
    Sprite* sprite = new Sprite;
    sprite->Initialize(spriteCommon, "Resources/uvChecker.png");

    // 3Dモデルの初期化
    Model* model = new Model;;
    model->Initialize(ModelManager::GetInstance()->GetModelCommon(), "Resources", ModelPath01);

    // 3Dオブジェクトの初期化
    Object3d* object3d = new Object3d;
    object3d->Initialize(object3dCommon);
    // モデルを結びつける
    object3d->SetModel(ModelPath01);

#pragma endregion 最初のシーンの終了

    //リソースリークチェック
    D3DResourceLeakChecker leakCheck;

    std::vector<Sprite*> sprites;
    const uint32_t spritesize = 6;
    float spritesPosition[spritesize]{};
    for (uint32_t i = 0; i < spritesize; ++i) {
        spritesPosition[i] = 180.0f * i;
        Sprite* sprite = new Sprite();
        sprite->Initialize(spriteCommon, TexturePath01);
        if (i % 2 == 1) {
            sprite->SetTexture(TexturePath02);
        }
        // 現在の位置を取得
        Vector2 position = sprite->GetPosition();
        // 位置を変更する
        position.x = spritesPosition[i];
        position.y = 100.0f;
        // 変更した座標を設定
        sprite->SetPosition(position);
        // 情報を転送
        sprites.push_back(sprite);
    }

    std::vector<Object3d*> objects;
    const uint32_t objectize = 2;
    float objectsPosition[objectize] = { 2.0f, -2.0f };
    // 乱数のシードを設定
    srand(static_cast<unsigned int>(time(nullptr)));
    // サイコロを振る
    int dice = rand() % 6 + 1;

    for (uint32_t i = 0; i < objectize; ++i) {
        Object3d* object3d = new Object3d();
        object3d->Initialize(object3dCommon);
        if (i % 2 == 0) {
            object3d->SetModel(ModelPath02);
        } else {
            object3d->SetModel(ModelPath01);
        }

        // 位置を設定
        Vector3 position = object3d->GetTranslate();
        position.x = objectsPosition[i];
        object3d->SetTranslate(position);

        objects.push_back(object3d);
    }
    int time_ = 0;

    // ウィンドウの×ボタンが押されるまでループ
    while (true) {
        // Windowのメッセージ処理
        if (winApp->ProcessMessage()) {
            // ゲームループを抜ける
            break;
        }
        // ゲームの処理

        // 入力の更新
        input->Update();

        // 0を押している間true
        if (input->Pushkey(DIK_0)) {
            OutputDebugStringA("Hit 0 \n");
        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
        //ImGui::ShowDemoWindow();

        ImGui::Begin("Sprite");
        ImGui::DragInt("Dise",&dice);
        ImGui::End();

          //ImGuiの描画コマンドを生成
        ImGui::Render();

        /*-------------------------------------------------------------------------------------------------------*/
        /*-----------------------------------3Dオブジェクトの更新処理の開始------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/

        size_t index = 0;
        size_t maxIterations = 2;
        for (Object3d* object3d : objects) {
            if (index >= maxIterations) {
                break; // 指定回数を超えたらループを終了
            }
            object3d->Update();
            Vector3 rotation = object3d->GetRotate();
            if (index == 0) {
                rotation.z += 0.01f;
            } else if (index == 1) {
                rotation.y += 0.01f;
            }
            object3d->SetRotate(rotation);
            // インクリメントして次へ
            ++index;
        }

        /*-------------------------------------------------------------------------------------------------------*/
        /*-----------------------------------3Dオブジェクトの更新処理の終了------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/


        /*--------------------------------------------------------------------------------------------------------*/
        /*---------------------------------------Spriteの更新処理の開始----------------------------------------------*/
        /*-------------------------------------------------------------------------------------------------------*/


        // 更新処理
        for (Sprite* sprite : sprites) {
            sprite->Update();
        }

        /*-------------------------------------------------------------------------------------------------------*/
        /*-------------------------------------Spriteの更新処理の終了----------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------*/

        // 描画用のDescriptorHeapの設定
        ID3D12DescriptorHeap* descriptorHeap[] = { dxCommon->GetsrvDescriptorHeap().Get() };
        dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeap);

        //  DirectXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
        dxCommon->PreDraw();

        /*------------------------------------------------------------------------------------------------------*/
        /*----------------------------------3Dオブジェクトの描画処理開始--------------------------------------------*/
        /*-----------------------------------------------------------------------------------------------------*/

        // 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
        object3dCommon->Commondrawing();
#pragma region 全てのObject3d個々の描画

        // コールバック関数を設定
        PFunc p = DispResult;
        setTimeout(p, dice, time_, objects);

#pragma endregion 全てのObject3d個々の描画
        /*------------------------------------------------------------------------------------------------------*/
        /*----------------------------------3Dオブジェクトの描画処理終了--------------------------------------------*/
        /*-----------------------------------------------------------------------------------------------------*/

        /*----------------------------------------------------------------------------------------------------*/
        /*------------------------------------Spriteの描画処理開始----------------------------------------------*/
        /*---------------------------------------------------------------------------------------------------*/

        // Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
        spriteCommon->Commondrawing();
#pragma region 全てのSprite個々の描画

        // スプライトの描画（再帰関数を利用）
        DrawSpritesRecursively(sprites, 0);

#pragma endregion 全てのSprite個々の描画
        /*----------------------------------------------------------------------------------------------------*/
        /*------------------------------------Spriteの描画処理終了----------------------------------------------*/
        /*---------------------------------------------------------------------------------------------------*/

        //実際のcommandListのImGuiの描画コマンドを積む
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList().Get());

        // 描画後処理
        dxCommon->PostDrow();
    }

#pragma region 各処理の解放


    // シーンの解放
    delete  spriteCommon;
    delete  object3dCommon;
    // 汎用機能の解放

    // スプライトの解放
    delete  sprite;
    for (Sprite* sprite : sprites) {
        delete sprite;
    }
    // 3Dモデルの解放
    delete model;
    // 3Dオブジェクトの解放
    delete  object3d;
    for (Object3d* object3d : objects) {
        delete object3d;
    }
    // 入力解放
    delete input;

    // テクスチャマネージャーの終了
    TextureManager::GetInstance()->Finalize();
    // 3Dモデルマネージャの終了
    ModelManager::GetInstance()->Finalize();
    // DirectXの解放
    delete dxCommon;

    // ウィンドウ解放 
    // WindowsAPIの終了処理
    winApp->Finalize();
    // WindowsAPIの解放
    delete winApp;
#pragma endregion 各処理の解放
    return 0;
}