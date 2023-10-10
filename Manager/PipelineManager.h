#pragma once
#include "../base/WinApp.h"
#include "../base/DirectXCommon.h"
#include "../object/Triangle.h"
#include "../Manager/ImGuiManager.h"
#include "../Manager/TextureManager.h"
#include "../components/light/Light.h"
#include "../object/Sprite.h"
#include "../object/Sphere.h"

enum BlendMode {
	// ブレンドなし
	kBlendModeNone,
	// 通常αブレンド
	kBlendModeNormal,
	// 加算
	kBlendModeAdd,
	// 減算
	kBlendModeSubtract,
	// 乗算
	kBlendModeMultiply,
	// スクリーン
	kBlendModeScreen,
	// 利用してはいけない
	kCountOfBlendMode,
};

class PipelineManager {
public:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDsvDescriptorHeap() { return dsvDescriptorHeap_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetDepthStencilResource() { return depthStencilResource_.Get(); }
	D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc() { return depthStencilDesc_; }
	Microsoft::WRL::ComPtr<ID3D12PipelineState>* GetGraphicsPipelineState() { return graphicsPipelineState_; }
	Microsoft::WRL::ComPtr<ID3D12RootSignature>* GetRootSignature() { return rootSignature_; }

	static PipelineManager* GetInstance();

	// DXCの初期化
	void DXCInitialize();

	// シェーダのコンパイル
	IDxcBlob* CompileShader(
		// CompilerするShaderファイルへのパス
		const std::wstring& filePath,
		// Compilerに使用するProfile
		const wchar_t* profile,
		// 初期化で生成したものを3つ
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);

	// DepthStenciltextureの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(int32_t width, int32_t height);

	// dsvDescriptorHeapの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDsvDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// CPUで書き込む用のTextureResourceを作りコマンドを積む
	void CreateDepthStencilView();

	// DepthStencilStateの設定
	void SettingDepthStencilState();

	// DescriptorRangeの生成
	void CreateDescriptorRange();

	// DescriptorTableの生成
	void CraeteDescriptorTable();

	// Samplerの設定
	void SettingSampler();

	// RootSignatureの生成
	void CreateRootSignature();

	// RootParameter生成
	void CreateRootParameter();

	// InputLayerの設定
	void SettingInputLayout();

	// BlendStateの設定
	void SettingBlendState();

	// RasterizerStateの設定
	void SettingRasterizerState();

	// ピクセルシェーダー
	void PixelSharder();

	// 頂点シェーダー
	void VertexSharder();

	// PSOの生成
	void CreatePSO();

	// PSO
	void PSO();

	// ビューポート
	void CreateViewport();

	// シザー矩形
	void CreateScissor();

	~PipelineManager() = default;

	// エンジンの初期化
	void Initialize();

	// 描画前の処理
	void BeginFrame();

	// 描画後の処理
	void EndFrame();

private:
	IDxcUtils* dxcUtils_;
	IDxcCompiler3* dxcCompiler_;
	IDxcIncludeHandler* includeHandler_;

	ID3DBlob* errorBlob_;

	static const int kMaxPSO = 6;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_[kMaxPSO];
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[kMaxPSO][3];
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_[kMaxPSO];
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_[kMaxPSO];
	ID3DBlob* signatureBlob_[kMaxPSO];

	D3D12_RASTERIZER_DESC rasterizerDesc_[kMaxPSO];
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDescs_[kMaxPSO];
	D3D12_BLEND_DESC blendDesc_[kMaxPSO];
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[kMaxPSO];
	D3D12_ROOT_PARAMETER rootParameters_[kMaxPSO][5];
	D3D12_DESCRIPTOR_RANGE descriptorRange_[kMaxPSO][1];
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[kMaxPSO][1];

	IDxcBlob* vertexShaderBlob_;
	IDxcBlob* pixelShaderBlob_;

	D3D12_VIEWPORT viewport_;
	D3D12_RECT scissorRect_;

	// Depth
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
};
