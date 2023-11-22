#pragma once
#include "math/Matrix4x4.h"
#include "VertexData.h"
#include "Transform.h"
#include "base/WorldTransform.h"
#include "base/ViewProjection.h"
#include "Material.h"
#include  "Transform.h"
#include "base/MaterialData.h"
#include <random>
#include <d3d12.h>

struct Particle {
	Transform transform;
	Vector3 vel;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

// GPUに送る
struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

class Particles
{
public:
	///
	/// Default Method
	///

	// 初期化
	void Initialize();

	// 更新処理
	void Update(const ViewProjection& viewProjection);

	// 描画
	void Draw();

	ModelData GetModelData() { return modelData_; }
	///
	/// User Method
	/// 
	
	// particleの座標と速度のランダム生成
	Particle MakeNewParticle(std::mt19937& randomEngine);

	Vector3 KelvinToRGB(int kelvin);

	// ImGuiでパラメータをまとめたもの
	void ImGuiAdjustParameter();
private:

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);

	void CreateVertexResource();

	void CreateVertexBufferView();

	void CreateMaterialResource();

	// 線形補完
	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

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
	// 複数描画のための変数
	const static uint32_t kNumMaxInstance = 10;
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_;
	ParticleForGPU* instancingData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	
	Particle particles_[kNumMaxInstance];
	const float kDeltaTime = 1.0f / 60.0f;
	uint32_t numInstance = 0;
	
	ViewProjection viewProjection_[kNumMaxInstance];

	ModelData modelData_;

	Vector3 a = {0,0,0};
};

