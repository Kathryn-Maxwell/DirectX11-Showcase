// Tessellation pixel shader
// calculates per pixel normals based on height displacement
#include "VertexManipulation.hlsli"
#include "LightingCalculation.hlsli"

Texture2D heightMap : register(t0);
Texture2D albedo : register(t1);
Texture2D normalMap : register(t2);
SamplerState clampSampler : register(s0);
SamplerState wrapSampler : register(s1);


cbuffer LightBuffer : register(b0)
{
    LightData lightData[MAX_LIGHTS];
};

cbuffer HeightMapBuffer : register(b1)
{
    float maxHeight;
    int2 meshSize;
    float textureScale;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
};

float3 calculateNormal(float2 _uv)
{
    float2 textureDimensions = float2(0, 0);
    float val = 0.0f;
    
    heightMap.GetDimensions(0, textureDimensions.x, textureDimensions.y, val);
    float2 uvOffset = float2(1.0f / textureDimensions.x, 1.0f / textureDimensions.y);
    float2 worldStep = meshSize * uvOffset;
    
    float northHeight = getTerrainHeight(heightMap, clampSampler, float2(_uv.x, _uv.y + uvOffset.y), maxHeight);
    float southHeight = getTerrainHeight(heightMap, clampSampler, float2(_uv.x, _uv.y - uvOffset.y), maxHeight);
    float eastHeight = getTerrainHeight(heightMap, clampSampler, float2(_uv.x + uvOffset.x, _uv.y), maxHeight);
    float westHeight = getTerrainHeight(heightMap, clampSampler, float2(_uv.x - uvOffset.x, _uv.y), maxHeight);
    float height = getTerrainHeight(heightMap, clampSampler, _uv, maxHeight);
	
    float3 north = float3(0, northHeight - height, 1.0f * worldStep.y);
    float3 south = float3(0, southHeight - height, -1.0f * worldStep.y);
    float3 east = float3(1.0f * worldStep.x, eastHeight - height, 0);
    float3 west = float3(-1.0f * worldStep.x, westHeight - height, 0);
    
    float3 surfaceNormal = normalize(float3(cross(north, east) + cross(east, south) + cross(south, west) + cross(west, north)));
    //float3 normalMapSample = normalize((float3) normalMap.Sample(wrapSampler, _uv * textureScale));
    return normalize(surfaceNormal/* + normalMapSample*/);
}

float4 main(InputType _input) : SV_TARGET
{
	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    float4 textureColour = albedo.Sample(wrapSampler, _input.tex * textureScale);
    float4 lightColour = calculateLighting(lightData, calculateNormal(_input.tex), _input.worldPosition);

    //return heightMap.Sample(clampSampler, _input.tex);
    //return float4(calculateNormal(_input.tex), 1);
    return lightColour * textureColour;
}