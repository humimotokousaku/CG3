#include "SpotLight.h"
#include "Manager/ImGuiManager.h"
#include <assert.h>
#include <numbers>

SpotLight* SpotLight::GetInstance() {
	static SpotLight instance;

	return &instance;
}

SpotLight::~SpotLight() {

}

void SpotLight::Initialize() {
	CreateSpotLightResource();

	// spotLightingのデフォ値
	spotLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData_->pos = { 2.0f, 1.25f, 0.0f };
	spotLightData_->intensity = 1.0f;
	spotLightData_->decay = 1.0f;
	spotLightData_->direction = Normalize(Vector3{ -1.0f, -1.0f, 0.0f });
	spotLightData_->distance = 7.0f;
	spotLightData_->decay = 2.0f;
	spotLightData_->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLightData_->cosFalloffStart = std::cos(std::numbers::pi_v<float> / 3.0f);
}

void SpotLight::ImGuiAdjustParameter() {
	ImGui::DragFloat3("spotLighting.pos", &spotLightData_->pos.x,0.1f, -100, 100);
	ImGui::DragFloat("spotLighting.cosAngle", &spotLightData_->cosAngle,0.01f, 0, 6.28f);
	ImGui::DragFloat("spotLighting.cosFalloffStart", &spotLightData_->cosFalloffStart, 0.01f, 0, 6.28f);
	ImGui::DragFloat("spotLighting.distance", &spotLightData_->distance,0.1f, 0, 10);
	ImGui::DragFloat("spotLighting.decay", &spotLightData_->decay,0.1f, 0, 10);
	ImGui::DragFloat("spotLighting.intensity", &spotLightData_->intensity,0.1f, 0, 1);
	ImGui::ColorEdit3("spotLighting.color", &spotLightData_->color.x);
}

Microsoft::WRL::ComPtr<ID3D12Resource> SpotLight::CreateBufferResource(size_t sizeInBytes) {
	HRESULT hr;
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes; // リソースのサイズ。今回はVector4を3頂点分
	// バッファの場合はこれからは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// 実際に頂点リソースを作る
	hr = DirectXCommon::GetInstance()->GetDevice().Get()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;
}

void SpotLight::CreateSpotLightResource() {
	spotLightResource_ = CreateBufferResource(sizeof(SpotLightforGPU)).Get();
	// マテリアルにデータを書き込む
	spotLightData_ = nullptr;
	// 書き込むためのアドレスを取得
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
}




