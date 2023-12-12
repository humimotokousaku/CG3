#pragma once
#include "math/Vector4.h"
#include "math/Vector3.h"
#include <d3d12.h>
#include <wrl.h>

struct SpotLightforGPU {
	Vector4 color;
	Vector3 pos;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
	float padding[2];
};

class SpotLight
{
public:
	static SpotLight* GetInstance();

	~SpotLight();

	void Initialize();

	void ImGuiAdjustParameter();

	Microsoft::WRL::ComPtr<ID3D12Resource> GetSpotLightResource() { return spotLightResource_.Get(); }
	void SetSpotLightResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& spotLightResource) { spotLightResource_ = spotLightResource.Get(); }

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	void CreateSpotLightResource();



private:
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_;
	SpotLightforGPU* spotLightData_;
};