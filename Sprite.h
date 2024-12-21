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
private:
	// 頂点データ作成
	void VertexDatacreation();
	// リソース
	// マテリアル
	void MaterialGenerate();
	void TransformationMatrixGenerate();

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

	Vector2 position = { 0.0f,0.0f };
	float rotation = 0.0f;
	Vector2 size = { 90.0f,90.0f };
	// テクスチャ番号
	uint32_t textureindex = 0;
public: 
	// getter
	const Vector2& GetPosition() const { return position; }
	float GetRotation() const { return rotation; }
	const Vector4& GetColor() const { return materialData->color; }
	const Vector2& GetSize()const { return size; }	
	// setter
	void SetPosition(const Vector2& position) { this->position = position; }
	void SetRotation(float rotation) { this->rotation = rotation; }
	void SetColor(const Vector4& color) { materialData->color = color; }
	void SetSize(const Vector2& size) { this->size = size; }
};