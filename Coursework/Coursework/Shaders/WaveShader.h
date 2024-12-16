#pragma once
// Wave shader
// 

#define NUM_WAVES 4

#include "DefaultShader.h"

class TessellationPlaneMesh;

class WaveShader : public DefaultShader
{
	struct WaveData {
		XMFLOAT3 direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
		float phase = 0.0f;
		float time = 0.0f;
		float timeScale = 1.0f;
		float gravity = 9.81f;
		float depth = 10.0f;
		float amplitude = 0.0f;
		XMFLOAT3 padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	};

	struct WaveDataBufferType {
		WaveData waveData[NUM_WAVES];
	};

	struct WaveTessellationBufferType {
		int maxTessellation = 6;
		int minTessellation = 2;
		float maxHeight = 1;
		float minHeight = -1;
	};

public:
	WaveShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera, Timer* _timer);
	~WaveShader();

	void renderMesh(TessellationPlaneMesh* _waterMesh, Matrices _matrices);

	void gui() override;

private:
	void initShader() override;
	void setShaderParameters(TessellationPlaneMesh* _waterMesh, Matrices _matrices);

	Timer* timer = nullptr;
	ID3D11Buffer* waveDataBuffer = nullptr;
	ID3D11Buffer* tessellationBuffer = nullptr;

	WaveData waveData[NUM_WAVES];

	XMINT2 tessellationRange = XMINT2(2, 6);
};