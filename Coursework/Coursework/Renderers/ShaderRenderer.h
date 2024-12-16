#pragma once
// Shader Renderer
// The renderer that renders the scenes geometry using the traditional render pipeline
//

#include "Renderer.h"

class ShaderRenderer : public Renderer
{
public:
	ShaderRenderer(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, Timer* _timer, FPCamera* _camera, XMINT2 _screenSize);
	~ShaderRenderer();

	RenderTexture* render() override;
	void renderWireFrame();
	void gui() override;

protected:
	void renderScene();
	RenderTexture* currentFrame;
};