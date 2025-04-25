#include "Camera.h"
#include "MatrixVector.h"
#include "WinApp.h"
#include<ImGuiManager.h>

using namespace MatrixVector;

Camera::Camera()
	: transform({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} })
	, fovY(0.45f)
	, aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip(0.1f)
	, farclip(100.0f)
	, worludMatrix(MakeAftineMatrix(transform.scale, transform.rotate, transform.translate))
	, viewMatrix(Inverse(worludMatrix))
	, projectionMatrix(MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farclip))
	, ViewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
{
}

void Camera::Update() {
	worludMatrix = MakeAftineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = Inverse(worludMatrix);
	projectionMatrix = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farclip);
	ViewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
}

void Camera::DebugUpdata() {
#ifdef USE_IMGUI
	// 開発用UIの処理
	ImGui::Begin("Camera");
	ImGui::DragFloat3("Translate", &transform.translate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.0001f);
	ImGui::End();
#endif // USE_IMGUI
}