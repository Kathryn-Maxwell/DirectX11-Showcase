// Path trace pixel shader
// Recives the SphereData, CameraData, and the RayTracingSettings and uses them to cast rays into the scene
// Functionalty should be moved to a compute shader instead

#include "PathTracing.hlsli"

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraDataBuffer : register(b1)
{
    // Used
    uint2 screenSize;
    float2 planeSize;   
        
    float4 cameraLocalPosition;
    
    float3 cameraForward;
    float nearClipPlane;
    
    float3 cameraRight;
    float padding;
}

cbuffer MeshDataBuffer : register(b2)
{
    SphereData sphereData[MAX_SPHERES];
};

cbuffer RayTracingSettings : register(b3)
{
    int maxRayBounce;
    int raysPerPixel;
    int multiPassFramesRendered;
    int sphereCount;
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
    uint randomSeed = GenerateRandomSeed(_input.tex, screenSize, multiPassFramesRendered);
    
    float3 cameraWorldPosition = (float3) mul(cameraLocalPosition, worldMatrix);
    float3 viewPoint = CalcualteViewPointLocation(_input.tex, planeSize, cameraForward, cameraRight, cameraWorldPosition, nearClipPlane);
    
    
    Ray ray;
    ray.origin = cameraWorldPosition;
    ray.direction = normalize(viewPoint - ray.origin);
    //return float4(ray.direction, 1);       
    
    float3 incomingLight = float3(0, 0, 0);
    
    for (int rayIndex = 0; rayIndex < raysPerPixel; rayIndex++)
    {
        incomingLight += Trace(ray, maxRayBounce, sphereData, sphereCount, randomSeed);
    }

    float3 pixelColour = incomingLight / raysPerPixel;  
    return float4(pixelColour, 1);
}    