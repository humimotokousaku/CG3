#pragma once
#include "math/Vector4.h"
#include "math/Vector3.h"
#include <d3d12.h>
#include <wrl.h>

struct PointLightforGPU {
	Vector4 color;
	Vector3 pos;
	float intensity;
	float radius;
	float decay;
	float padding[2];
};

class PointLight
{
public:
	static PointLight* GetInstance();

	Microsoft::WRL::ComPtr<ID3D12Resource> GetPointLightResource() { return pointLightResource_.Get(); }
	void SetPointLightResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& pointLightResource) { pointLightResource_ = pointLightResource.Get(); }

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	void CreatePointLightResource();

	~PointLight();

	void Initialize();

	void ImGuiAdjustParameter();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_;
	PointLightforGPU* pointLightData_;
};