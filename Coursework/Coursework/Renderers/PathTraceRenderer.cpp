#include "PathTraceRenderer.h"
#include "../Shaders/PathTraceShader.h"
#include "../Shaders/MultiPassRenderShader.h"

PathTraceRenderer::PathTraceRenderer(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, Timer* _timer, FPCamera* _camera, XMINT2 _screenSize) :
	Renderer(_renderer, _hwnd, _lightManager, _textureManager, _timer, _camera, _screenSize)
{
	pathTraceShader = new PathTraceShader(renderer, _hwnd, lightManager, textureManager, camera, screenSize, &spheres);
	multiPassRenderShader = new MultiPassRenderShader(renderer, _hwnd, lightManager, textureManager, camera);

	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenSize.x, screenSize.y, 0, 0);
	currentFrame = new RenderTexture(renderer->getDevice(), screenSize.x, screenSize.y, SCREEN_NEAR, SCREEN_DEPTH);
	multiPassFrame01 = new RenderTexture(renderer->getDevice(), screenSize.x, screenSize.y, SCREEN_NEAR, SCREEN_DEPTH);
	multiPassFrame02 = new RenderTexture(renderer->getDevice(), screenSize.x, screenSize.y, SCREEN_NEAR, SCREEN_DEPTH);
	multiPassFrame = multiPassFrame01;
	lastFrame = multiPassFrame02;
}

PathTraceRenderer::~PathTraceRenderer()
{
	delete multiPassFrame02;
	multiPassFrame02 = nullptr;

	delete multiPassFrame01;
	multiPassFrame01 = nullptr;

	delete currentFrame;
	currentFrame = nullptr;

	delete orthoMesh;
	orthoMesh = nullptr;

	delete multiPassRenderShader;
	multiPassRenderShader = nullptr;
	
	delete pathTraceShader;
	pathTraceShader = nullptr;

	Renderer::~Renderer();
}

RenderTexture* PathTraceRenderer::render()
{
	camera->setPosition(25, 5.5f, 30.0f);
	camera->setRotation(0, 0, 0);

	// Generate the view matrix based on the camera's position.
	camera->update();

	renderCurrentFrame();

	if (multiPassRenderShader->isMultiPassRendering())
	{
		renderMultiPassFrame();
		return multiPassFrame;
	}

	return currentFrame;
}

void PathTraceRenderer::renderCurrentFrame() const
{
	ID3D11DeviceContext* deviceContext = renderer->getDeviceContext();

	currentFrame->setRenderTarget(deviceContext);
	currentFrame->clearRenderTarget(deviceContext, 0.2f, 0.2f, 0.2f, 1.0f);

	Matrices matrices = Matrices(renderer->getWorldMatrix(), camera->getOrthoViewMatrix(), renderer->getOrthoMatrix());

	// Render ortho mesh
	renderer->setZBuffer(false);
	pathTraceShader->render(orthoMesh, matrices, multiPassRenderShader->isMultiPassRendering());
	renderer->setZBuffer(true);
}

void PathTraceRenderer::renderMultiPassFrame()
{
	ID3D11DeviceContext* deviceContext = renderer->getDeviceContext();

	if (multiPassFrame == multiPassFrame01)
	{
		multiPassFrame = multiPassFrame02;
		lastFrame = multiPassFrame01;
	}
	else
	{
		multiPassFrame = multiPassFrame01;
		lastFrame = multiPassFrame02;
	}

	multiPassFrame->setRenderTarget(deviceContext);
	multiPassFrame->clearRenderTarget(deviceContext, 0.6f, 0.6f, 0.6f, 1.0f);

	Matrices matrices = Matrices(renderer->getWorldMatrix(), camera->getOrthoViewMatrix(), renderer->getOrthoMatrix());

	// Render ortho mesh
	renderer->setZBuffer(false);
	multiPassRenderShader->render(orthoMesh, matrices, currentFrame, lastFrame);
	renderer->setZBuffer(true);
}

void PathTraceRenderer::gui()
{
	pathTraceShader->gui();
	multiPassRenderShader->gui();
}