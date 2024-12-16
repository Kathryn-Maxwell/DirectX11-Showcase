// Terrain vertex shader.
// Pass forward data, strip out some values not required.

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
};

OutputType main(InputType _input)
{
    OutputType output;

	 // Pass the vertex position into the hull shader.
    output.position = _input.position;
    
    // Pass the input color into the hull shader.
    output.tex = _input.tex;
    
    return output;
}