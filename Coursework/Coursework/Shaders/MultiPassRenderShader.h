#pragma once
// Multi Pass Render Shader
//
//

#include "DefaultShader.h"

class LightManager;
class TextureManager;

class MultiPassRenderShader : public DefaultShader
{
	struct MultiPassRenderSettingsBufferType 
	{
		float multiPassFramesRendered;
		XMFLOAT3 padding;
	};

public:
	MultiPassRenderShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera);
	~MultiPassRenderShader();

	void render(BaseMesh* _mesh, const Matrices _matrices, RenderTexture* _currentFrame, RenderTexture* _lastFrame);
	
	void resetMultiPassRendering();

	void gui() override;

	bool isMultiPassRendering() { return multiPassRendering; }
	int getmultiPassFrameCount() { return multiPassFramesRendered; }

private:
	void initShader() override;
	void setShaderParameters(const Matrices _matrices, RenderTexture* _currentFrame, RenderTexture* _lastFrame);

	ID3D11Buffer* multiPassRenderSettingsBuffer = nullptr;

	int multiPassFramesRendered = 0;
	bool multiPassRendering = false;
};