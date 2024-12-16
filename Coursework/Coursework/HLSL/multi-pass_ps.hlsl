Texture2D lastFrame : register(t0);
Texture2D currentFrame : register(t1);
SamplerState sampler0 : register(s0);

cbuffer MultiPassRenderSettings : register(b0)
{
    float multiPassFramesRendered;
    float3 padding;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

float4 main(InputType _input) : SV_TARGET
{
    float4 lastFramePixelColour = lastFrame.Sample(sampler0, _input.tex);
    float4 currentFramePixelColour = currentFrame.Sample(sampler0, _input.tex);
        
    if (multiPassFramesRendered == 0)
    {
        return currentFramePixelColour;
    }
        
    //return currentFramePixelColour + lastFramePixelColour;
    
    float weight = 1.0 / (multiPassFramesRendered + 1);
    float4 newPixelColour = lastFramePixelColour * (1 - weight) + currentFramePixelColour * weight;
    
    return newPixelColour;
}