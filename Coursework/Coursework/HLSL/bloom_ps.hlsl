#include "PostProcessing.hlsli"

Texture2D previousSample : register(t0);
Texture2D accumulatedFrame : register(t1);
SamplerState sampler0 : register(s0);

cbuffer BloomSettingsBuffer : register(b0)
{
    float bloomRenderState;
    float threshold;    
    float sampleScale;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

float4 preFilter(float4 _colour, float2 _uv)
{
    _colour -= float4(threshold, threshold, threshold, 1);
    _colour = max(0, _colour);
    return _colour;  
}

float4 main(InputType _input) : SV_TARGET
{    
    float2 textureDimensions = float2(0, 0);
    float val = 0.0f;
    
    previousSample.GetDimensions(0, textureDimensions.x, textureDimensions.y, val);
    float2 texelSize = float2(1.0f / textureDimensions.x, 1.0f / textureDimensions.y);
    
    float4 colour = previousSample.Sample(sampler0, _input.tex);
    
    // Pre-Filter
    if (bloomRenderState == 0)
    {
        return preFilter(colour, _input.tex);
    }
    
    // Down Sample
    if (bloomRenderState == 1)
    {
        return DownsampleBox13Tap(previousSample, sampler0, _input.tex, texelSize);
    }
    
    // Up Sameple & Combine
    if (bloomRenderState == 2)
    {
        float4 accumulatedPixelColour = accumulatedFrame.Sample(sampler0, _input.tex);
        float4 upSampleColour = upSampleTent(previousSample, sampler0, _input.tex, texelSize, sampleScale);
        float4 pixelColour = min(accumulatedPixelColour + upSampleColour, 1);
        return pixelColour;
    }
    
    // Combine
    if (bloomRenderState == 3)
    {
        float4 accumulatedPixelColour = accumulatedFrame.Sample(sampler0, _input.tex);
        float4 upSampleColour = previousSample.Sample(sampler0, _input.tex);
        float4 pixelColour = min(accumulatedPixelColour + upSampleColour, 1);
        return pixelColour;
    }
    
    // Should never be hit, as one of the three states should be enabled
        return colour;
}