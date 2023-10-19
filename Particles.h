#pragma once
#include "math/Matrix4x4.h"
#include "VertexData.h"
#include "Transform.h"
#include "base/WorldTransform.h"
#include "base/ViewProjection.h"
#include "Material.h"
#include "base/MaterialData.h"
#include <d3d12.h>

class Particles
{
public:
	///
	/// Default Method
	///

	// 初期化
	void Initialize();

	// 更新処理
	//void Update();

	// 描画
	void Draw(const ViewProjection& viewProjection, int blendNum);

	ModelData GetModelData() { return modelData_; }
	///
	/// User Method
	/// 
	
	// ImGuiでパラメータをまとめたもの
	void ImGuiAdjustParameter();
private:

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);

	void CreateVertexResource();

	void CreateVertexBufferView();

	void CreateMaterialResource();

private:
	// Material
	Material* materialData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Transform uvTransform_;
	Matrix4x4 uvTransformMatrix_;
	// Vertex
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	VertexData* vertexData_;
	const static int kMaxParticle = 10;
	WorldTransform worldTransform_[kMaxParticle];

	//const int instanceCount = 10;

	ModelData modelData_;
};

