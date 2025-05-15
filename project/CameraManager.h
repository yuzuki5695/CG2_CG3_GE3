//#pragma once
//#pragma once
//#include <Camera.h>
//#include <memory>
//#include<wrl.h>
//
//class CameraManager
//{
//private:
//    static std::unique_ptr<CameraManager> instance;
//
//    CameraManager(CameraManager&) = delete;
//    CameraManager& operator=(CameraManager&) = delete;
//public: // メンバ関数
//    CameraManager() = default;
//    ~CameraManager() = default;
//
//    // シングルトンインスタンスの取得
//    static CameraManager* GetInstance();
//    // 終了
//    void Finalize();
//
//    // 初期化
//    void Initialize();
//    // 更新
//    void Update();
//
//    // 追従対象を設定
//    void SetFollowTarget(Transform* target);
//    void SetDefaultCamera();
//
//    // アクティブカメラ取得
//    Camera* GetCurrentCamera() const { return currentCamera_; }
//
//private:
//    // メンバ変数
//    Camera defaultCamera_;
//    Camera followCamera_;
//    Camera* currentCamera_ = nullptr;
//
//    Transform* followTarget_ = nullptr;
//};