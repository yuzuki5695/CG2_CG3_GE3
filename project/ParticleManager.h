#pragma once
#include<DirectXCommon.h>
#include<SrvManager.h>
#include<random>
#include<Vector2.h>
#include<Vector3.h>
#include<Vector4.h>
#include<Matrix4x4.h>

// パーティクルマネージャ
class ParticleManager
{
private:

	static ParticleManager* instance;

	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(ParticleManager&) = delete;
	ParticleManager& operator=(ParticleManager&) = delete;
public:
	// 頂点データ
	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	// マテリアル
	struct Material {
		Vector4 color;
		int32_t endbleLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};
	// マテリアルデータ
	struct MaterialDate {
		std::string textureFilePath;
		uint32_t textureindex = 0;
	};
	// モデルデータ
	struct ModelDate {
		std::vector<VertexData> vertices;
		MaterialDate material;
	};
private:
	// ルートシグネチャの生成
	void RootSignatureGenerate();
	// グラフィックスパイプラインの生成
	void GraphicsPipelineGenerate();
	// 頂点データ作成
	void VertexDatacreation();
public: // メンバ関数
	// シングルトンインスタンスの取得
	static ParticleManager* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* birectxcommon, SrvManager* srvmanager);
private: // メンバ変数
	// ポインタ
	DirectXCommon* dxCommon_;
	SrvManager* srvmanager_;
	// ランダムエンジン
	std::mt19937 randomEngine;
	// RootSignature
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
	// Objファイルのデータ
	ModelDate modelDate;
	// バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResoruce;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;






};