#pragma once
#include"Transform.h"
#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include "Matrix4x4.h"
#include<d3d12.h>
#include<wrl.h>
#include<cstdint>
#include<fstream>

class SpriteCommon;

class Sprite
{
public: // メンバ関数
	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct Material {
		Vector4 color;
		int32_t endbleLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};


	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

public: // メンバ関数
	// 初期化
	void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);
	// 更新処理
	void Update();
	// 描画処理
	void Draw();

	void SetTexture(const std::string& textureFilePath);
private:
	// 頂点データ作成
	void VertexDatacreation();
	// リソース
	// マテリアル
	void MaterialGenerate();
	void TransformationMatrixGenerate();
	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

private: // メンバ変数
	// ポインタ
	SpriteCommon* spriteCommon_ = nullptr;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce;
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResource;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	// Transform  uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Vector2 position = { 100.0f,100.0f };
	float rotation = 0.0f;
	Vector2 size = { 360.0f,360.0f };
	// テクスチャ番号
	uint32_t textureindex = 0;
	// アンカーポイント
	Vector2 anchorPoint = { 0.0f,0.0f };
	// 左右フリップ
	bool isFlipX_ = false;
	// 上下フリップ
	bool isFlipY_ = false;
	// テクスチャ左上座標
	Vector2 textureLeftTop = { 0.0f,0.0f };
	// テクスチャ切り出しサイズ
	Vector2 textureSize = { 64.0f,64.0f };
public: 
	// getter
	const Vector2& GetPosition() const { return position; }
	float GetRotation() const { return rotation; }
	const Vector4& GetColor() const { return materialData->color; }
	const Vector2& GetSize()const { return size; }
	const Vector2& GetAnchorPoint()const { return anchorPoint; }
	const bool& GetisFlipX()const { return isFlipX_; }
	const bool& GetisFlipY()const { return isFlipY_; }
	const Vector2& GetTextureLeftTop()const { return textureLeftTop; }
	const Vector2& GetTextureSize()const { return textureSize; }
	// setter
	void SetPosition(const Vector2& position) { this->position = position; }
	void SetRotation(float rotation) { this->rotation = rotation; }
	void SetColor(const Vector4& color) { materialData->color = color; }
	void SetSize(const Vector2& size) { this->size = size; }
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }
	void SetisFlipX(const bool& isFlipX) { this->isFlipX_ = isFlipX; }
	void SetisFlipY(const bool& isFlipY) { this->isFlipY_ = isFlipY; }
	void SetTextureLeftTop(const Vector2& textureLeftTop) { this->textureLeftTop = textureLeftTop; }
	void SetTextureSize(const Vector2& textureSize) { this->textureSize = textureSize; }
};