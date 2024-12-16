#include "ShaderRenderer.h"

#include "../LightManager.h"
#include "../TessellationPlaneMesh.h"
#include "../Shaders/TerrainShader.h"
#include "../Shaders/WaveShader.h"

ShaderRenderer::ShaderRenderer(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, Timer* _timer, FPCamera* _camera, XMINT2 _screenSize) :
	Renderer(_renderer, _hwnd, _lightManager, _textureManager, _timer, _camera, _screenSize)
{
	// Shaders
	terrainShader = new TerrainShader(renderer, _hwnd, lightManager, textureManager, camera);
	waveShader = new WaveShader(renderer, _hwnd, lightManager, textureManager, camera, timer);

	currentFrame = new RenderTexture(renderer->getDevice(), screenSize.x, screenSize.y, SCREEN_NEAR, SCREEN_DEPTH);
}

ShaderRenderer::~ShaderRenderer()
{
	delete currentFrame;
	currentFrame = nullptr;

	delete waveShader;
	waveShader = nullptr;

	delete terrainShader;
	terrainShader = nullptr;

	Renderer::~Renderer();
}

RenderTexture* ShaderRenderer::render()
{
	ID3D11DeviceContext* deviceContext = renderer->getDeviceContext();
	currentFrame->setRenderTarget(deviceContext);
	currentFrame->clearRenderTarget(deviceContext, 0.2f, 0.2f, 0.2f, 1.0f);

	renderScene();

	return currentFrame;
}

void ShaderRenderer::renderWireFrame()
{
	// Set render target to buffer (renderers render to a RenderTexture)
	renderer->setBackBufferRenderTarget();
	// Clear the scene. (default grey colour)
	renderer->beginScene(0.2f, 0.2f, 0.2f, 1.0f);

	renderScene();
}

void ShaderRenderer::renderScene()
{
	ID3D11DeviceContext* deviceContext = renderer->getDeviceContext();

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	Matrices matrices = Matrices(renderer->getWorldMatrix(), camera->getViewMatrix(), renderer->getProjectionMatrix());

	terrainShader->renderMesh(terrainMesh, matrices);

	for (size_t cube = 0; cube < cubes.size(); cube++)
	{
		defaultShader->renderMesh(cubes[cube], matrices, textureManager->getTexture(L"checker_board"));
	}

	for (size_t sphere = 0; sphere < spheres.size(); sphere++)
	{
		defaultShader->renderMesh(spheres[sphere], matrices, textureManager->getTexture(L"checker_board"));
	}

	renderer->setAlphaBlending(true);
	waveShader->renderMesh(waterMesh, matrices);
	renderer->setAlphaBlending(false);

	lightManager->render(deviceContext, defaultShader, matrices);
}

void ShaderRenderer::gui()
{
	Renderer::gui();

	terrainShader->gui();
	waveShader->gui();
}
