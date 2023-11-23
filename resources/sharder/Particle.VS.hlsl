#include "Particle.hlsli"

struct ParticleForGPU {
    float32_t4x4 matWorld;
    float32_t4x4 WVP;
    float32_t4 color;
};
struct ViewProjectionMatrix {
    float32_t4x4 view;
    float32_t4x4 projection;
    float32_t3 cameraPos;
};
StructuredBuffer<ParticleForGPU> gParticle : register(t0);
ConstantBuffer<ViewProjectionMatrix> gViewProjectionMatrix : register(b1);

struct VertexShaderInput {
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID) {
    VertexShaderOutput output;

    output.position = mul(input.position, gParticle[instanceId].WVP);
    output.texcoord = input.texcoord;
    output.color = gParticle[instanceId].color;
    return output;
}