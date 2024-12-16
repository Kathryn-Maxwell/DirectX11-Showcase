#define NUM_WAVES 4

// Samples the height map and returns the target y displacement
float getTerrainHeight(Texture2D _texture, SamplerState _sampler, float2 _uv, float _maxHeight)
{
    float height = _texture.SampleLevel(_sampler, _uv, 0).r;
    return lerp(0.0f, _maxHeight, height);
}

struct WaveData
{    
    float3 direction;
    float phase;
    
    float time;
    float timeScale;
    float gravity;
    float depth;
    
    float amplitude;
    float3 padding;
};

float frequency(WaveData _waveData)
{
    float directionLength = length(_waveData.direction);
    return sqrt((_waveData.gravity * directionLength) * tanh(_waveData.depth * directionLength));
}

float theta(float3 _position, WaveData _waveData)
{
    return (_waveData.direction.x * _position.x) + (_waveData.direction.z * _position.z) - (frequency(_waveData) * _waveData.time * _waveData.timeScale) - _waveData.phase;
}

// Returns the displacment from a wave
float3 getGerstnerWavePoint(float3 _position, WaveData _waveData)
{
    float _theta = theta(_position, _waveData);
    float directionLength = length(_waveData.direction);
    
    float x = (sin(_theta) * ((_waveData.direction.x / directionLength) * (_waveData.amplitude / tanh(directionLength * _waveData.depth)))) * -1;
    float y = cos(_theta) * _waveData.amplitude;
    float z = (sin(_theta) * ((_waveData.direction.z / directionLength) * (_waveData.amplitude / tanh(directionLength * _waveData.depth)))) * -1;
    
    return float3(x, y, z);
}

// Returns the total displacment from all waves
float3 getGerstnerWaveDisplacement(float3 _position, WaveData _waveData[NUM_WAVES])
{
    float3 displacement = float3(0.0f, 0.0f, 0.0f);
    for (int wave = 0; wave < NUM_WAVES; wave++)
    {
        displacement += getGerstnerWavePoint(_position, _waveData[wave]);
    }
    return displacement;
}