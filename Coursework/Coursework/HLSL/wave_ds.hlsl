// Wave domain shader
// After tessellation the domain shader processes the all the vertices

#include "VertexManipulation.hlsli"

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer WaveDataBuffer : register(b1)
{
    WaveData waveData[NUM_WAVES];
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
    //float4 position : POSITION;
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 waveWorldPosition : TEXCOORD1;
};

float3 calculateNormal(float3 _vertexWorldPosition, float3 _waveWorldPosition)
{
    float3 up = _vertexWorldPosition + float3(0, 0, 1);
    float3 right = _vertexWorldPosition + float3(1, 0, 0);
    
    float3 upNeighbour = up + getGerstnerWaveDisplacement(up, waveData);
    float3 rightNeighbour = right + getGerstnerWaveDisplacement(right, waveData);
    
    return normalize(cross(normalize(upNeighbour - _waveWorldPosition), normalize(rightNeighbour - _waveWorldPosition)));
}

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
    
    //float3 vertexWorldPosition = mul(float4(vertexPosition, 1.0f), worldMatrix).xyz;
    float3 vertexWorldPosition = vertexPosition;
    
    // Vertex manipulation
    vertexPosition += getGerstnerWaveDisplacement(vertexPosition, waveData);    
    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    //output.position = float4(vertexPosition, 1.0f);
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.waveWorldPosition = mul(float4(vertexPosition, 1.0f), worldMatrix).xyz;
    
    output.normal = normalize(mul(calculateNormal(vertexWorldPosition, output.waveWorldPosition), (float3x3) worldMatrix));
    //output.normal = calculateNormal(vertexWorldPosition, output.waveWorldPosition);

    output.tex = uvPosition;
    return output;
}