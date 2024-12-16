// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices

#include "VertexManipulation.hlsli"

Texture2D heightMap : register(t0);
SamplerState clampSampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer HeightMapBuffer : register(b1)
{
    float maxHeight;
    int2 meshSize;
    float textureScale;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
};

[domain("quad")]
OutputType main(ConstantOutputType _input, float2 _uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> _patch)
{
    OutputType output;
 
    // Determine the position of the new vertex.		
    float3 v1 = lerp(_patch[0].position, _patch[1].position, _uvwCoord.y);
    float3 v2 = lerp(_patch[3].position, _patch[2].position, _uvwCoord.y);
    float3 vertexPosition = lerp(v1, v2, _uvwCoord.x);
    
    // Determine the UVs of the new vertex.
    float2 uv1 = lerp(_patch[0].tex, _patch[1].tex, _uvwCoord.y);
    float2 uv2 = lerp(_patch[3].tex, _patch[2].tex, _uvwCoord.y);
    float2 uvPosition = lerp(uv1, uv2, _uvwCoord.x);
    
    // Vertex manipulation
    vertexPosition.y += getTerrainHeight(heightMap, clampSampler, uvPosition, maxHeight);
    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.worldPosition = mul(float4(vertexPosition, 1.0f), worldMatrix).xyz;
    
    output.tex = uvPosition;
    return output;
}