
// Code from: https://github.com/Unity-Technologies/Graphics/blob/master/com.unity.postprocessing/PostProcessing/Shaders/Sampling.hlsl
// Better, temporally stable box filtering
// [Jimenez14] http://goo.gl/eomGso
// . . . . . . .
// . A . B . C .
// . . D . E . .
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .
float4 DownsampleBox13Tap(Texture2D _texture, SamplerState _sampler, float2 _uv, float2 _texelSize)
{
    float4 A = _texture.Sample(_sampler, _uv + _texelSize * float2(-1.0, -1.0));
    float4 B = _texture.Sample(_sampler, _uv + _texelSize * float2(0.0, -1.0));
    float4 C = _texture.Sample(_sampler, _uv + _texelSize * float2(1.0, -1.0));
    float4 D = _texture.Sample(_sampler, _uv + _texelSize * float2(-0.5, -0.5));
    float4 E = _texture.Sample(_sampler, _uv + _texelSize * float2(0.5, -0.5));
    float4 F = _texture.Sample(_sampler, _uv + _texelSize * float2(-1.0, 0.0));
    float4 G = _texture.Sample(_sampler, _uv);
    float4 H = _texture.Sample(_sampler, _uv + _texelSize * float2(1.0, 0.0));
    float4 I = _texture.Sample(_sampler, _uv + _texelSize * float2(-0.5, 0.5));
    float4 J = _texture.Sample(_sampler, _uv + _texelSize * float2(0.5, 0.5));
    float4 K = _texture.Sample(_sampler, _uv + _texelSize * float2(-1.0, 1.0));
    float4 L = _texture.Sample(_sampler, _uv + _texelSize * float2(0.0, 1.0));
    float4 M = _texture.Sample(_sampler, _uv + _texelSize * float2(1.0, 1.0));

    float2 div = (1.0 / 4.0) * half2(0.5, 0.125);

    float4 o = (D + E + I + J) * div.x;
    o += (A + B + G + F) * div.y;
    o += (B + C + H + G) * div.y;
    o += (F + G + L + K) * div.y;
    o += (G + H + M + L) * div.y;
    return o;
}

// Code from: https://github.com/Unity-Technologies/Graphics/blob/master/com.unity.postprocessing/PostProcessing/Shaders/Sampling.hlsl
// 9-tap bilinear upsampler (tent filter)
float4 upSampleTent(Texture2D _texture, SamplerState _sampler, float2 _uv, float2 _texelSize, float4 _sampleScale)
{
    float4 d = _texelSize.xyxy * float4(1.0, 1.0, -1.0, 0.0) * _sampleScale;

    float4 sample;
    sample = _texture.Sample(_sampler, _uv - d.xy);
    sample += _texture.Sample(_sampler, _uv - d.wy) * 2.0;
    sample += _texture.Sample(_sampler, _uv - d.zy);

    sample += _texture.Sample(_sampler, _uv + d.zw) * 2.0;
    sample += _texture.Sample(_sampler, _uv) * 4.0;
    sample += _texture.Sample(_sampler, _uv + d.xw) * 2.0;

    sample += _texture.Sample(_sampler, _uv + d.zy);
    sample += _texture.Sample(_sampler, _uv + d.wy) * 2.0;
    sample += _texture.Sample(_sampler, _uv + d.xy);

    return sample * (1.0 / 16.0);
}