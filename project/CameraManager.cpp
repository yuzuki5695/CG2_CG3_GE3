//#include "CameraManager.h"
//
//// 静的メンバ変数の定義
//std::unique_ptr<CameraManager> CameraManager::instance = nullptr;
//
//// シングルトンインスタンスの取得
//CameraManager* CameraManager::GetInstance() {
//    if (!instance) {
//        instance = std::make_unique<CameraManager>();
//    }
//    return instance.get();
//}
//
//// 終了
//void CameraManager::Finalize() {
//    instance.reset();  // `delete` 不要
//}
//
//void CameraManager::Initialize() {
//    currentCamera_ = &defaultCamera_;
//}
//
//void CameraManager::Update() {
//    if (currentCamera_ == &followCamera_ && followTarget_) {
//        followCamera_.transform.translate = followTarget_->translate;
//    }
//    currentCamera_->Update();
//}
//
//void CameraManager::SetFollowTarget(Transform* target) {
//    followTarget_ = target;
//    followCamera_.SetTarget(target);  // カメラに追従先を設定
//    currentCamera_ = &followCamera_;
//}
//
//void CameraManager::SetDefaultCamera() {
//    followCamera_.SetTarget(nullptr); // 追従解除
//    followTarget_ = nullptr;
//    currentCamera_ = &defaultCamera_;
//}
