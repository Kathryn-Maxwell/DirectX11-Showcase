#define MAX_LIGHTS 10

struct Attenuation
{
    float Constant;
    float Linear;
    float Exponential;
    float padding;
};

struct LightData
{
    int type;
    int3 typePadding;
    
    float4 position;
    float4 direction;
    
    float4 ambient;
    float4 diffuse;
    float4 specular;
    
    Attenuation attenuation;
};

float4 calculateDirectionLighting(LightData _lightData, float3 _normal)
{
    float intensity = saturate(dot(_normal, (float3)_lightData.direction));
    return saturate(_lightData.ambient + _lightData.diffuse * intensity);
}

float4 calculatePointLight(LightData _lightData, float3 _normal, float3 _worldPosition)
{
    float3 lightVector = (float3) _lightData.position - _worldPosition;
    float lightDistance = length(lightVector);
    float3 lightDirection = normalize(lightVector);
    float intensity = saturate(dot(_normal, lightDirection));
    float4 colour = saturate(_lightData.diffuse * intensity);

    float attenuationValue =
		_lightData.attenuation.Constant +
		_lightData.attenuation.Linear * lightDistance +
		_lightData.attenuation.Exponential * lightDistance * lightDistance;

    return colour / attenuationValue;
}

float4 calculateLighting(LightData _lightData[MAX_LIGHTS], float3 _normal, float3 _worldPosition)
{
    float4 lightColour = float4(0, 0, 0, 0);
    
    for (int light = 0; light < MAX_LIGHTS; light++)
    {
        switch ((int)_lightData[light].type.x)
        {
            // Off
            case 0:
                break;
            
            // Directional
            case 1:
                lightColour += calculateDirectionLighting(_lightData[light], _normal);
                break;
            
            // Point
            case 2:
                lightColour += calculatePointLight(_lightData[light], _normal, _worldPosition);
                break;
            
            // Spot
            case 3:
                break;            
        }      
    }
    
    return lightColour;
}