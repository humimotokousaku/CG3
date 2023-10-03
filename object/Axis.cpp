#include "Axis.h"
#include "../components/camera/Camera.h"
#include "../Manager/ImGuiManager.h"
#include "../Manager/ObjManager.h"
#include "../utility/GlobalVariables.h"
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>

Microsoft::WRL::ComPtr<ID3D12Resource> Axis::CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes) {
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
	hr = device.Get()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;
}

void Axis::CreateVertexResource() {
	vertexResource_ = CreateBufferResource(DirectXCommon::GetInstance()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size()).Get();
}

void Axis::CreateVertexBufferView() {
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * UINT(modelData_.vertices.size());
	// 1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Axis::CreateMaterialResource() {
	materialResource_ = CreateBufferResource(DirectXCommon::GetInstance()->GetDevice(), sizeof(Material)).Get();
	// マテリアルにデータを書き込む
	materialData_ = nullptr;
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
}

void Axis::CreateWvpResource() {
	// 1つ分のサイズを用意する
	transformationMatrixResource_ = CreateBufferResource(DirectXCommon::GetInstance()->GetDevice(), sizeof(TransformationMatrix)).Get();
	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	// 単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
}

void Axis::Initialize() {
	// モデルを読み込み
	modelData_ = ObjManager::GetInstance()->GetObjModelData()[AXIS];

	CreateVertexResource();

	CreateMaterialResource();

	CreateWvpResource();

	CreateVertexBufferView();

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	uvTransform_ = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	// Lightingするか
	materialData_->enableLighting = true;

	// uvTransform行列の初期化
	materialData_->uvTransform = MakeIdentity4x4();

	GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	const char* groupName = "Axis";
	GlobalVariables::GetInstance()->CreateGroup(groupName);
	globalVariables->AddItem(groupName, "Translation", transform_.translate);
	globalVariables->AddItem(groupName, "Scale", transform_.scale);
	globalVariables->AddItem(groupName, "Rotate", transform_.rotate);
	globalVariables->AddItem(groupName, "Color", materialData_->color);
}

void Axis::Draw() {
	ApplyGlobalVariables();
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	// ボタンを押したらsave
	if (globalVariables->GetInstance()->GetIsSave()) {
		globalVariables->SaveFile("Axis");
	}

	if (isAlive_) {
		uvTransformMatrix_ = MakeScaleMatrix(uvTransform_.scale);
		uvTransformMatrix_ = Multiply(uvTransformMatrix_, MakeRotateZMatrix(uvTransform_.rotate.z));
		uvTransformMatrix_ = Multiply(uvTransformMatrix_, MakeTranslateMatrix(uvTransform_.translate));
		materialData_->uvTransform = uvTransformMatrix_;

		// カメラ
		//transform_.rotate.y += 0.006f;
		transformationMatrixData_->World = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
		transformationMatrixData_->WVP = Multiply(transformationMatrixData_->World, *Camera::GetInstance()->GetTransformationMatrixData());
		transformationMatrixData_->World = MakeIdentity4x4();

		materialData_->color = { 1.0f,1.0f,1.0f,1.0f };


		// コマンドを積む
		DirectXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
		// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
		DirectXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// DescriptorTableの設定
		DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetTextureSrvHandleGPU()[2]);

		// wvpのCBufferの場所を設定
		DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_.Get()->GetGPUVirtualAddress());

		DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, Light::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());

		// マテリアルCBufferの場所を設定
		DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_.Get()->GetGPUVirtualAddress());
		DirectXCommon::GetInstance()->GetCommandList()->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
	}
}

void Axis::Release() {

}

void Axis::ApplyGlobalVariables() {
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	const char* groupName = "Axis";
	transform_.translate = globalVariables->GetVector3Value(groupName, "Translation");
	transform_.scale = globalVariables->GetVector3Value(groupName, "Scale");
	transform_.rotate = globalVariables->GetVector3Value(groupName, "Rotate");
	materialData_->color = globalVariables->GetVector4Value(groupName, "Color");
}

void Axis::ImGuiAdjustParameter() {
	//ImGui::Checkbox("isAlive", &isAlive_);
	//ImGui::CheckboxFlags("isLighting", &materialData_->enableLighting, 1);
	//ImGui::SliderFloat3("Translate", &transform_.translate.x, -5, 5);
	//ImGui::SliderFloat3("Scale", &transform_.scale.x, -5, 5);
	//ImGui::SliderFloat3("Rotate", &transform_.rotate.x, -6.28f, 6.28f);
	//ImGui::Text("UvTransform");
	//ImGui::SliderFloat2("UvTranslate", &uvTransform_.translate.x, -5, 5);
	//ImGui::SliderFloat2("UvScale", &uvTransform_.scale.x, -5, 5);
	//ImGui::SliderAngle("UvRotate.z", &uvTransform_.rotate.z);
}