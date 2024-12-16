#include "PostProcessingRenderer.h"

#include <cmath>
#include "../Shaders/BloomShader.h"

PostProcessingRenderer::PostProcessingRenderer(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, Timer* _timer, FPCamera* _camera, XMINT2 _screenSize) :
	Renderer(_renderer, _hwnd, _lightManager, _textureManager, _timer, _camera, _screenSize)
{
	bloomShader = new BloomShader(renderer, _hwnd, lightManager, textureManager, camera);

	currentFrame = new RenderTexture(renderer->getDevice(), screenSize.x, screenSize.y, SCREEN_NEAR, SCREEN_DEPTH);
	
	for (int sampleLevel = 0; sampleLevel < MAX_BLOOM_SAMPLES + 1; sampleLevel++)
	{
		XMINT2 sampleFrameSize = XMINT2(screenSize.x / pow(2, sampleLevel), screenSize.y / pow(2, sampleLevel));
		bloomDownSamples.push_back(new RenderTexture(renderer->getDevice(), sampleFrameSize.x, sampleFrameSize.y, SCREEN_NEAR, SCREEN_DEPTH));
		bloomUpSamples.push_back(new RenderTexture(renderer->getDevice(), sampleFrameSize.x, sampleFrameSize.y, SCREEN_NEAR, SCREEN_DEPTH));
	}

	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenSize.x, screenSize.y, 0, 0);
}

PostProcessingRenderer::~PostProcessingRenderer()
{
	delete orthoMesh;
	orthoMesh = nullptr;

	for (int bloomDownSample = 0; bloomDownSample < bloomDownSamples.size(); bloomDownSample++)
	{
		delete bloomDownSamples[bloomDownSample];
		bloomDownSamples[bloomDownSample] = nullptr;
	}

	for (int bloomUpSample = 0; bloomUpSample < bloomUpSamples.size(); bloomUpSample++)
	{
		delete bloomUpSamples[bloomUpSample];
		bloomUpSamples[bloomUpSample] = nullptr;
	}
	
	delete currentFrame;
	currentFrame = nullptr;

	delete bloomShader;
	bloomShader = nullptr;

	Renderer::~Renderer();
}

RenderTexture* PostProcessingRenderer::render(RenderTexture* _PreProcessedFrame)
{
	currentFrame = _PreProcessedFrame;

	if(bloomShader->isBloomEnabled()) renderBloom();

	return currentFrame;
}

void PostProcessingRenderer::renderBloom()
{
	ID3D11DeviceContext* deviceContext = renderer->getDeviceContext();
	Matrices matrices = Matrices(renderer->getWorldMatrix(), camera->getOrthoViewMatrix(), renderer->getOrthoMatrix());

	renderer->setZBuffer(false);

	// pre filter the current frame render and output to the first downsample render texture
	bloomDownSamples[0]->setRenderTarget(deviceContext);
	bloomShader->render(orthoMesh, matrices, currentFrame, nullptr, BloomShader::BloomState::PRE_FILTER);

	// For each subsequent render pass down sample the previous result
	for (int bloomSample = 1; bloomSample < bloomShader->getBloomSamplesCount(); bloomSample++)
	{
		bloomDownSamples[bloomSample]->setRenderTarget(deviceContext);
		bloomShader->render(orthoMesh, matrices, bloomDownSamples[bloomSample - 1], nullptr, BloomShader::BloomState::DOWN_SAMPLE);
	}

	// Have to clear the last render texture so it's empty for the [bloomSample + 1] portion of the next loop
	bloomUpSamples[bloomShader->getBloomSamplesCount()]->clearRenderTarget(deviceContext, 0, 0, 0, 1);

	// For each downsampled render, upsample and combine the appropiate down sample result
	for (int bloomSample = bloomShader->getBloomSamplesCount() - 1; bloomSample >= 0; bloomSample--)
	{
		bloomUpSamples[bloomSample]->setRenderTarget(deviceContext);
		bloomShader->render(orthoMesh, matrices, bloomDownSamples[bloomSample], bloomUpSamples[bloomSample + 1], BloomShader::BloomState::UP_SAMPLE_AND_COMBINE);
	}

	// Reuse this render texture as it's not needed this frame anymore.
	// Combine the bloom result to the current frame
	bloomDownSamples[0]->setRenderTarget(deviceContext);
	bloomShader->render(orthoMesh, matrices, currentFrame, bloomUpSamples[0], BloomShader::BloomState::COMBINE);

	currentFrame = bloomDownSamples[0];

	renderer->setZBuffer(true);
}

void PostProcessingRenderer::gui()
{
	if (ImGui::CollapsingHeader("Post Processing"))
	{
		if (ImGui::TreeNode("Bloom"))
		{
			bloomShader->gui();
			ImGui::TreePop();
		}
	}
}