// Tessellation Hull Shader
// Prepares control points for tessellation

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer DynamicTessellationBuffer : register(b1)
{
    float3 tessellationTarget;    
    float tessellationStrength;
};

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
};

// Calculates the tessellation factor based on the vertices distance to the target
int CalculateLOD(float3 _position)
{
    float4 worldPosition = mul(float4(_position, 1.0f), worldMatrix);
    
    float distanceToTarget = distance(tessellationTarget, (float3)worldPosition);
    return max(tessellationStrength - round(distanceToTarget), 1);
}

float3 ComputePatchMidPoint(float3 _position1, float3 _position2, float3 _position3, float3 _position4)
{
    return (_position1 + _position2 + _position3 + _position4) * 0.25f;
}

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 12> _inputPatch, uint _patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

    // Determine the midpoint of the patches
    float3 midPoints[5] =
    {
        // Main quad
        ComputePatchMidPoint(_inputPatch[0].position, _inputPatch[1].position, _inputPatch[2].position, _inputPatch[3].position),
        // +x neighbor
        ComputePatchMidPoint(_inputPatch[2].position, _inputPatch[3].position, _inputPatch[4].position, _inputPatch[5].position),
        // +z neighbor
        ComputePatchMidPoint(_inputPatch[0].position, _inputPatch[3].position, _inputPatch[6].position, _inputPatch[7].position),
        // -x neighbor
        ComputePatchMidPoint(_inputPatch[1].position, _inputPatch[0].position, _inputPatch[8].position, _inputPatch[9].position),
        // -z neighbor
        ComputePatchMidPoint(_inputPatch[1].position, _inputPatch[2].position, _inputPatch[10].position, _inputPatch[11].position)
    };
    
    // Determine the appropriate LOD for this patch
    float LODs[5] =
    {
        // Main quad
        CalculateLOD(midPoints[0]),
        // +x neighbor
        CalculateLOD(midPoints[1]),
        // +z neighbor
        CalculateLOD(midPoints[2]),
        // -x neighbor
        CalculateLOD(midPoints[3]),
        // -z neighbor
        CalculateLOD(midPoints[4])
    };
    
    // Set the tessellation factors for the four edges of the quad.
    output.edges[0] = min(LODs[0], LODs[3]); // Left
    output.edges[1] = min(LODs[0], LODs[2]); // Up
    output.edges[2] = min(LODs[0], LODs[1]); // Right
    output.edges[3] = min(LODs[0], LODs[4]); // Down

    // Set the tessellation factor for tessallating inside the quad.
    output.inside[0] = LODs[0];
    output.inside[1] = LODs[0];

    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 12> _patch, uint _pointId : SV_OutputControlPointID, uint _patchId : SV_PrimitiveID)
{
    OutputType output;

    // Set the position for this control point as the output position.
    output.position = _patch[_pointId].position;

    // Set the input colour as the output colour.
    output.tex = _patch[_pointId].tex;

    return output;
}