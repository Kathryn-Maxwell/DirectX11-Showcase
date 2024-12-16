// Default geometry shader Draws normals per vetex
// Not currently enabled as the default_vs output needs to be changed to feed into this input, other rendering using the default vertex shader would not compile as they won't have the GS enabled
// Was used for debugging the post tessellation wave normals 

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

[maxvertexcount(6)]
void main(triangle InputType input[3], inout LineStream<OutputType> lStream)
{
    OutputType output;
    for (int i = 0; i < 3; i++)
    {
        output.position = mul(float4(input[i].position, 1.0f), worldMatrix);
        output.position = mul(output.position, viewMatrix);
        output.position = mul(output.position, projectionMatrix);
        //output.position = input[i].position;
        output.tex = input[i].tex;
        output.worldPosition = input[i].worldPosition;
        output.normal = normalize(mul(input[i].normal, (float3x3) worldMatrix));
        //output.normal = input[i].normal;
        lStream.Append(output);
        
        output.position = mul(float4(input[i].position.xyz + input[i].normal * 0.5f, 1), worldMatrix);
        output.position = mul(output.position, viewMatrix);
        output.position = mul(output.position, projectionMatrix);
        lStream.Append(output);
        lStream.RestartStrip();
    }
}