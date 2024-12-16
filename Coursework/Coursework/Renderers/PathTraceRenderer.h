#pragma once
// Path Trace Renderer
// The renderer resposible for rendering the ray traced scene in the pixel shader
//

#include "Renderer.h"

class OrthoMesh;
class PathTraceShader;
class MultiPassRenderShader;

#pragma once
class PathTraceRenderer : public Renderer
{
public:
	PathTraceRenderer(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, Timer* _timer, FPCamera* _camera, XMINT2 _screenSize);
	~PathTraceRenderer();

	RenderTexture* render() override;
	void gui() override;

protected:
	void renderCurrentFrame() const;
	void renderMultiPassFrame();

	OrthoMesh* orthoMesh = nullptr;
	RenderTexture* currentFrame = nullptr;
	RenderTexture* multiPassFrame01 = nullptr;
	RenderTexture* multiPassFrame02 = nullptr;
	RenderTexture* multiPassFrame = nullptr;
	RenderTexture* lastFrame = nullptr;

	PathTraceShader* pathTraceShader = nullptr;
	MultiPassRenderShader* multiPassRenderShader = nullptr;
};