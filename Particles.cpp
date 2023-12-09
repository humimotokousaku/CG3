#include "Particles.h"
#include "components/camera/Camera.h"
#include "Manager/ImGuiManager.h"
#include "utility/GlobalVariables.h"
#include "Manager/PipelineManager.h"
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>
#include <numbers>

void Particles::Initialize(bool isRandomColor = false, bool isRandomLifeTime = false) {
	// 頂点の座標
	modelData_.vertices.push_back({ .position = {-1.0f,1.0f,0.0f,1.0f}, .texcoord = {0.0f,0.0f},.normal = {0.0f,0.0f,1.0f} }); // 左上
	modelData_.vertices.push_back({ .position = {1.0f,1.0f,0.0f,1.0f}, .texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} }); // 右上
	modelData_.vertices.push_back({ .position = {-1.0f,-1.0f,0.0f,1.0f}, .texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} }); // 左下
	modelData_.vertices.push_back({ .position = {-1.0f,-1.0f,0.0f,1.0f}, .texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} }); // 左下
	modelData_.vertices.push_back({ .position = {1.0f,1.0f,0.0f,1.0f}, .texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} }); // 右上
	modelData_.vertices.push_back({ .position = {1.0f,-1.0f,0.0f,1.0f}, .texcoord = {1.0f,1.0f},.normal = {0.0f,0.0f,1.0f} }); // 右下
	//modelData_.material.textureFilePath = "./resources/uvChecker.png";

	// Resource作成
	instancingResource_ = CreateBufferResource(DirectXCommon::GetInstance()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance);
	instancingData_ = nullptr;
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	// SRVの作成
	uint32_t descriptorSizeSRV = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	// SRVを作成するDescriptorHeapの場所を決める
	instancingSrvHandleCPU_ = DirectXCommon::GetInstance()->GetCPUDescriptorHandle(DirectXCommon::GetInstance()->GetSrvDescriptorHeap(), descriptorSizeSRV, 3);
	instancingSrvHandleGPU_ = DirectXCommon::GetInstance()->GetGPUDescriptorHandle(DirectXCommon::GetInstance()->GetSrvDescriptorHeap(), descriptorSizeSRV, 3);
	// SRVの生成
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU_);

	// 頂点データのメモリ確保
	CreateVertexResource();
	CreateVertexBufferView();

	// マテリアルのメモリ確保
	CreateMaterialResource();

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

	uvTransform_ = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	// Lightingするか
	//materialData_->enableLighting = false;

	materialData_->color = { 1.0f,1.0f,1.0f,1.0f };
	// uvTransform行列の初期化
	materialData_->uvTransform = MakeIdentity4x4();

	// エミッターの設定
	emitter_.count = 10;
	emitter_.frequency = 1;
	emitter_.frequencyTime = 0.0f;

	// フィールド(疑似風を作成)
	accField_.acc = { 15,0,0 };
	accField_.area.min = { -10,-10,-10 };
	accField_.area.max = { 10,10,10 };
	accField_.isActive = true;
}

void Particles::Update() {
	uint32_t numInstance = 0;
	for (std::list<Particle>::iterator particleIterator = particles_.begin(); particleIterator != particles_.end();) {
		// 生存時間が過ぎたら処理を行わない
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles_.erase(particleIterator);
			continue;
		}

		if (numInstance < kNumMaxInstance) {
			// fieldの範囲内のparticleには加速度を適用
			if (accField_.isActive) {
				if (IsCollision(accField_.area, (*particleIterator).transform.translate)) {
					(*particleIterator).vel = Add((*particleIterator).vel, Multiply(kDeltaTime, accField_.acc));
				}
			}
			// 移動処理
			(*particleIterator).transform.translate = Add((*particleIterator).transform.translate, Multiply(kDeltaTime, (*particleIterator).vel));

			// 指定した時間に透明になる
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
			(*particleIterator).color.w = alpha;
			instancingData_[numInstance].color = (*particleIterator).color;
			++numInstance;
		}

		++particleIterator;
	}

	emitter_.frequencyTime += kDeltaTime;
	if (emitter_.frequency <= emitter_.frequencyTime) {
		std::random_device seedGenerator;
		std::mt19937 randomEngine(seedGenerator());
		particles_.splice(particles_.end(), Emission(emitter_, randomEngine));
		emitter_.frequencyTime -= emitter_.frequency;
	}
}

void Particles::Draw(const ViewProjection& viewProjection, int textureNum) {
	// カメラ行列
	Matrix4x4 cameraMatrix = MakeAffineMatrix(Vector3{ 1,1,1 }, viewProjection.rotation_, viewProjection.translation_);
	// 板ポリを正面に向ける
	Matrix4x4 backToFrontMatrix = MakeIdentity4x4();
	// billboardMatrixを作成
	Matrix4x4 billboardMatrix = cameraMatrix;
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;

	uint32_t numInstance = 0;
	for (std::list<Particle>::iterator particleIterator = particles_.begin(); particleIterator != particles_.end();) {
		// 生存時間が過ぎたら処理を行わない
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles_.erase(particleIterator);
			continue;
		}

		if (numInstance < kNumMaxInstance) {

			// WVPとworldMatrixの計算
			Matrix4x4 worldMatrix = AffineMatrix((*particleIterator).transform.scale, billboardMatrix, (*particleIterator).transform.translate);//MakeAffineMatrix(particles_[index].transform.scale, Vector3{ 1,1,1 }/*particles_[index].transform.rotate*/, particles_[index].transform.translate);
			instancingData_[numInstance].World = Multiply(worldMatrix, Multiply(viewProjection.matView, viewProjection.matProjection));
			instancingData_[numInstance].WVP = instancingData_[numInstance].World;
			++numInstance;

			// 時間を進める
			(*particleIterator).currentTime += kDeltaTime;
		}

		++particleIterator;
	}

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootSignature(PipelineManager::GetInstance()->GetRootSignature()[6].Get());
	DirectXCommon::GetInstance()->GetCommandList()->SetPipelineState(PipelineManager::GetInstance()->GetGraphicsPipelineState()[6].Get()); // PSOを設定

	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	DirectXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// コマンドを積む
	DirectXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定

	// DescriptorTableの設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU_);
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetTextureSrvHandleGPU()[textureNum]);
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, Light::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());

	// マテリアルCBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	DirectXCommon::GetInstance()->GetCommandList()->DrawInstanced(6, numInstance, 0, 0);
}

bool Particles::IsCollision(const AABB& aabb, const Vector3& point) {
	if ((aabb.min.x <= point.x && aabb.max.x >= point.x) &&
		(aabb.min.y <= point.y && aabb.max.y >= point.y) &&
		(aabb.min.z <= point.z && aabb.max.z >= point.z)
		) {
		return true;
	}
	return false;
}

Particle Particles::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {
	// 座標
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	// 速度
	std::uniform_real_distribution<float> distVel(-1.0f, 1.0f);
	// 色
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	// 生存可能時間
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	Particle particle;
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0,0,0 };
	Vector3 randomTranslate;

	randomTranslate = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.transform.translate = Add(translate, randomTranslate);
	//if()
	particle.vel = { distribution(randomEngine) ,distribution(randomEngine) ,distribution(randomEngine) };
	particle.color = { distColor(randomEngine),distColor(randomEngine) ,distColor(randomEngine),1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;
	return particle;
}

std::list<Particle> Particles::Emission(const Emitter& emitter, std::mt19937& randomEngine) {
	std::list<Particle> particles;
	for (uint32_t count = 0; count < emitter.count; ++count) {
		particles.push_back(MakeNewParticle(randomEngine, emitter_.transform.translate));
	}
	return particles;
}

void Particles::ImGuiAdjustParameter() {
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	ImGui::Begin("Particles");
	if (ImGui::Button("Add Particle")) {
		particles_.splice(particles_.end(), Emission(emitter_, randomEngine));
	}
	ImGui::Text("Emitter.frquencyTime:%f", emitter_.frequencyTime);
	ImGui::DragFloat3("Emitter.Translate", &emitter_.transform.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::Checkbox("isFieldAcceleration", &accField_.isActive);
	ImGui::End();
}

Vector3 Particles::KelvinToRGB(int kelvin) {
	Vector3 color{};

	float temp = kelvin / 100.0f;
	float red, green, blue;

	if (temp <= 66) {
		red = 255;
		green = temp;
		green = 99.4708025861f * log(green) - 161.1195681661f;

		if (temp <= 19)
			blue = 0;
		else {
			blue = temp - 10;
			blue = 138.5177312231f * log(blue) - 305.0447927307f;
		}
	}
	else {
		red = temp - 60;
		red = 329.698727446f * pow(red, -0.1332047592f);

		green = temp - 60;
		green = 288.1221695283f * pow(green, -0.0755148492f);

		blue = 255;
	}

	color.x = red / 255.0f;
	color.y = green / 255.0f;
	color.z = blue / 255.0f;

	return color;
}

// 線形補完
Vector3 Particles::Lerp(const Vector3& v1, const Vector3& v2, float t) {
	return  Add(v1, Multiply(t, Subtract(v2, v1)));
}

Matrix4x4 Particles::AffineMatrix(const Vector3& scale, const Matrix4x4& rotateMatrix, const Vector3& translate) {
	// 計算結果
	Matrix4x4 result{};

	// アフィン変換行列の計算
	result.m[0][0] = scale.x * rotateMatrix.m[0][0];
	result.m[0][1] = scale.x * rotateMatrix.m[0][1];
	result.m[0][2] = scale.x * rotateMatrix.m[0][2];
	result.m[0][3] = 0.0f;

	result.m[1][0] = scale.y * rotateMatrix.m[1][0];
	result.m[1][1] = scale.y * rotateMatrix.m[1][1];
	result.m[1][2] = scale.y * rotateMatrix.m[1][2];
	result.m[1][3] = 0.0f;

	result.m[2][0] = scale.z * rotateMatrix.m[2][0];
	result.m[2][1] = scale.z * rotateMatrix.m[2][1];
	result.m[2][2] = scale.z * rotateMatrix.m[2][2];
	result.m[2][3] = 0.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Particles::CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes) {
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

void Particles::CreateVertexResource() {
	vertexResource_ = CreateBufferResource(DirectXCommon::GetInstance()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size()).Get();
}

void Particles::CreateVertexBufferView() {
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * UINT(modelData_.vertices.size());
	// 1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Particles::CreateMaterialResource() {
	materialResource_ = CreateBufferResource(DirectXCommon::GetInstance()->GetDevice(), sizeof(Material) * kNumMaxInstance).Get();
	// マテリアルにデータを書き込む
	materialData_ = nullptr;
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
}
