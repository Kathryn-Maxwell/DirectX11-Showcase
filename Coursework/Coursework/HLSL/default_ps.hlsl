// Default pixel shader, will sample the texture colour and apply the lighting calculations

#include "LightingCalculation.hlsli"
#include "VertexManipulation.hlsli"

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    LightData lightData[MAX_LIGHTS];
};

cbuffer SettingsBuffer : register(b1)
{
    bool renderTexture;
    bool renderLights;
    bool renderNormals;
    int padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

float4 main(InputType _input) : SV_TARGET
{
	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    float4 textureColour = renderTexture ? texture0.Sample(sampler0, _input.tex) : float4(1, 1, 1, 1);
    float4 lightColour = renderLights ? calculateLighting(lightData, _input.normal, _input.worldPosition) : float4(1, 1, 1, 1);
    float4 normalColour = renderNormals ? float4(_input.normal, 1) : float4(1, 1, 1, 1);
    
    return lightColour * textureColour * normalColour;
}