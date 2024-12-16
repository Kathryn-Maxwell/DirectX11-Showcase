#pragma once
// Post Processing Renderer
// The renderer responsible for rendering the bloom post process effect
// returns the render result as a rendertexture to be rendered onto the back buffer

#include "Renderer.h"

class OrthoMesh;
class BloomShader;

#pragma once
class PostProcessingRenderer : public Renderer
{
public:
	PostProcessingRenderer(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, Timer* _timer, FPCamera* _camera, XMINT2 _screenSize);
	~PostProcessingRenderer();

	RenderTexture* render(RenderTexture* _PreProcessedFrame);
	void gui() override;	

protected:
	void renderBloom();

	BloomShader* bloomShader = nullptr;

	OrthoMesh* orthoMesh = nullptr;
	
	RenderTexture* currentFrame = nullptr;

	RenderTexture* bloomFramePreFiltering = nullptr;
	std::vector<RenderTexture*> bloomDownSamples;
	std::vector<RenderTexture*> bloomUpSamples;

	float guiFloat = 0;
};