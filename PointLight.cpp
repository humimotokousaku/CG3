#include "PointLight.h"
#include "Manager/ImGuiManager.h"
#include <assert.h>

PointLight* PointLight::GetInstance() {
	static PointLight instance;

	return &instance;
}

PointLight::~PointLight() {

}

void PointLight::Initialize() {
	CreatePointLightResource();

	// PointLightingのデフォ値
	pointLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	pointLightData_->pos = { 0.0f, -0.5f, 0.0f };
	pointLightData_->intensity = 2.0f;
	pointLightData_->radius = 10.0f;
	pointLightData_->decay = 7.1f;
}

Microsoft::WRL::ComPtr<ID3D12Resource> PointLight::CreateBufferResource(size_t sizeInBytes) {
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

void PointLight::CreatePointLightResource() {
	pointLightResource_ = CreateBufferResource(sizeof(PointLightforGPU)).Get();
	// マテリアルにデータを書き込む
	pointLightData_ = nullptr;
	// 書き込むためのアドレスを取得
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));
}



void PointLight::ImGuiAdjustParameter() {
	ImGui::DragFloat3("PointLighting.pos", &pointLightData_->pos.x,0.1f, -100, 100);
	ImGui::ColorEdit3("PointLighting.color", &pointLightData_->color.x);
	ImGui::DragFloat("PointLighting.intensity", &pointLightData_->intensity, 0.01f, 0, 10);
	ImGui::DragFloat("PointLighting.radius", &pointLightData_->radius, 0.1f, 0, 50);
	ImGui::DragFloat("PointLighting.decay", &pointLightData_->decay, 0.01f, 0, 10);
}
