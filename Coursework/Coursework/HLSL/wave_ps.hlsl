// Wave pixel shader
// calculates per pixel normals based on wave displacement

#include "VertexManipulation.hlsli"
#include "LightingCalculation.hlsli"

cbuffer LightBuffer : register(b0)
{
    LightData lightData[MAX_LIGHTS];
};

cbuffer WaveDataBuffer : register(b1)
{
    WaveData waveData[NUM_WAVES];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 waveWorldPosition : TEXCOORD1;
};

float4 main(InputType _input) : SV_TARGET
{
    float4 colour = float4(0.0f, 0.502f, 0.502f, 0.8f);  
    float4 lightingColour = calculateLighting(lightData, _input.normal, _input.waveWorldPosition);
    
    //return float4(_input.normal, 1);
    return colour * float4(lightingColour.xyz, 1);
}