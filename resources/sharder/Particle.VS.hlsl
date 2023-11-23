#include "Particle.hlsli"

struct TransformationMatrix {
    float32_t4x4 matWorld;
    float32_t4x4 WVP;
};
struct ViewProjectionMatrix {
    float32_t4x4 view;
    float32_t4x4 projection;
    float32_t3 cameraPos;
};
StructuredBuffer<TransformationMatrix> gTransformationMatrices : register(t0);
ConstantBuffer<ViewProjectionMatrix> gViewProjectionMatrix : register(b1);

struct VertexShaderInput {
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID) {
    VertexShaderOutput output;
    //float32_t4x4 WorldViewProjectionMatrix = mul(gViewProjectionMatrix.view, gViewProjectionMatrix.projection);

    output.position = mul(input.position, gTransformationMatrices[instanceId].WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3)gTransformationMatrices[instanceId].matWorld));
    return output;
}