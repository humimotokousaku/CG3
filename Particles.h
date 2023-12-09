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

// Particleを発生させる
struct Emitter {
	Transform transform;
	uint32_t count;
	float frequency;
	float frequencyTime;
};

struct AABB {
	Vector3 min;
	Vector3 max;
};

struct AccField {
	Vector3 acc;  // 加速度
	AABB area;	  // 範囲
	bool isActive;// Fieldの活性化
};

class Particles
{
public:
	///
	/// Default Method
	///

	// 初期化
	void Initialize(bool isRandomColor, bool isRandomLifeTime);

	// 更新処理
	void Update();

	// 描画
	void Draw(const ViewProjection& viewProjection, int textureNum);

	ModelData GetModelData() { return modelData_; }

	///
	/// User Method
	///
	/// Setter

	// パーティクルの発生源の座標
	void SetEmitterPos(Vector3 translate) { emitter_.transform.translate = translate; }
	/// <summary>
	/// 一回で発生するパーティクルの数
	/// </summary>
	/// <param name="count">発生するパーティクルの数</param>
	void SetEmitterCount(float count) { emitter_.count = count; }
	/// <summary>
	/// パーティクルが発生する頻度
	/// </summary>
	/// <param name="frequency">秒</param>
	void SetEmitterFrequency(float frequency) { emitter_.frequencyTime = frequency; }

	// ImGuiでパラメータをまとめたもの
	void ImGuiAdjustParameter();

private:
	// particleの座標と速度のランダム生成
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	std::list<Particle> Emission(const Emitter& emitter, std::mt19937& randomEngine);

	bool IsCollision(const AABB& aabb, const Vector3& point);

	// 線形補完
	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

	Vector3 KelvinToRGB(int kelvin);

	Matrix4x4 AffineMatrix(const Vector3& scale, const Matrix4x4& rotateMatrix, const Vector3& translate);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);

	void CreateVertexResource();

	void CreateVertexBufferView();

	void CreateMaterialResource();

private:// 定数
	// 1フレームで進む時間
	const float kDeltaTime = 1.0f / 60.0f;
	// パーティクルの最大数
	const static uint32_t kNumMaxInstance = 100;

private:
	// パーティクル
	std::list<Particle> particles_;
	// エミッタ
	Emitter emitter_;
	// フィールド
	AccField accField_;

	ModelData modelData_;
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
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_;
	ParticleForGPU* instancingData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
};

