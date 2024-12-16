#pragma once
// Bloom Shader
//
//

#include "DefaultShader.h"

class BloomShader : public DefaultShader
{
	struct BloomSettingsBufferType
	{
		float bloomRenderState;
		float threshold;
		float sampleScale;
		float padding;
	};

public:
	enum class BloomState { PRE_FILTER, DOWN_SAMPLE, UP_SAMPLE_AND_COMBINE, COMBINE };

	BloomShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera);
	~BloomShader();

	void render(BaseMesh* _mesh, const Matrices _matrices, RenderTexture* _previousFrame, RenderTexture* _accumulatedFrame, BloomState _bloomState);

	void gui() override;

	bool isBloomEnabled() { return enabled; }
	int getBloomSamplesCount() { return bloomSamples; }

private:
	void initShader() override;
	void setShaderParameters(const Matrices _matrices, RenderTexture* _previousFrame, RenderTexture* _accumulatedFrame, BloomState _bloomState);

	ID3D11Buffer* bloomSettingsBuffer = nullptr;

	bool enabled = false;
	int bloomSamples = MAX_BLOOM_SAMPLES;
	float bloomThreshhold = 1;
	float sampleScale = 1;
};