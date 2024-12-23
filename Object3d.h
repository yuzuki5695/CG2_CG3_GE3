#pragma once
#include"Model.h"

class Object3dCommon;

//  3Dオブジェクト
class Object3d
{
public: // メンバ関数

	// 座標変換行列データ
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};
	// 平行光源データ
	struct DirectionalLight {
		Vector4 color; //!< ライトの色
		Vector3 direction; //!< ライトの向き
		float intensity; //!< 輝度
	};
public: // メンバ関数
	// 初期化
	void Initialize(Object3dCommon* object3dCommon);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();

private:
	// リソース
	// トランスフォームマトリックス
	void TransformationMatrixGenerate();
	// 平行光源リソース
	void DirectionalLightGenerate();
private:
	// ポインタ
	Object3dCommon* object3dCommon = nullptr;
	Model* model = nullptr;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource;
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;
	DirectionalLight* directionalLightDate = nullptr;

	Transform transform;
	Transform  cameraTransform;
public:
	// getter
	const Vector3& GetScale() const { return transform.scale; }
	const Vector3& GetRotate() const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }

	// setter
	void SetModel(Model* model) { this->model = model; }
	void SetScale(const Vector3& scale) { this->transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { this->transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { this->transform.translate = translate; }
};