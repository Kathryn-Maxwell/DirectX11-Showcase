#include "BloomShader.h"

BloomShader::BloomShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera)
: DefaultShader(_renderer, _hwnd, _lightManager, _textureManager, _camera)
{
	initShader();
}

BloomShader::~BloomShader()
{
	DefaultShader::~DefaultShader();
}

void BloomShader::initShader()
{
	// Load (+ compile) shader files
	loadVertexShader(L"default_vs.cso");
	loadPixelShader(L"bloom_ps.cso");

	createDefaultBuffers();

	D3D11_BUFFER_DESC bloomSettingsBufferDesc;
	bloomSettingsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bloomSettingsBufferDesc.ByteWidth = sizeof(BloomSettingsBufferType);
	bloomSettingsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bloomSettingsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bloomSettingsBufferDesc.MiscFlags = 0;
	bloomSettingsBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&bloomSettingsBufferDesc, NULL, &bloomSettingsBuffer);
}

void BloomShader::render(BaseMesh* _mesh, const Matrices _matrices, RenderTexture* _previousSample, RenderTexture* _accumulatedFrame, BloomState _bloomState)
{
	deviceContext = renderer->getDeviceContext();

	setShaderParameters(_matrices, _previousSample, _accumulatedFrame, _bloomState);
	_mesh->sendData(deviceContext);
	BaseShader::render(deviceContext, _mesh->getIndexCount());
}

void BloomShader::setShaderParameters(const Matrices _matrices, RenderTexture* _previousSample, RenderTexture* _accumulatedFrame, BloomState _bloomState)
{
	deviceContext = renderer->getDeviceContext();

	mapMatrixBuffer(deviceContext, _matrices);

	ID3D11ShaderResourceView* previousFrameTexture = _previousSample->getShaderResourceView();
	ID3D11ShaderResourceView* accumulatedFrameTexture = _accumulatedFrame == nullptr? previousFrameTexture : _accumulatedFrame->getShaderResourceView();

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(bloomSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	BloomSettingsBufferType* bloomSettingsDataPtr = (BloomSettingsBufferType*)mappedResource.pData;
	bloomSettingsDataPtr->bloomRenderState = (float)_bloomState;
	bloomSettingsDataPtr->threshold = bloomThreshhold;
	bloomSettingsDataPtr->sampleScale = sampleScale;
	bloomSettingsDataPtr->padding = 1;
	deviceContext->Unmap(bloomSettingsBuffer, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &bloomSettingsBuffer);
	deviceContext->PSSetShaderResources(0, 1, &previousFrameTexture);
	deviceContext->PSSetShaderResources(1, 1, &accumulatedFrameTexture);
	deviceContext->PSSetSamplers(0, 1, &clampSampler);
}

void BloomShader::gui()
{
	ImGui::Checkbox("Enable Bloom", &enabled);

	if (!enabled) return;

	ImGui::SliderInt("Bloom Samples", &bloomSamples, 1, MAX_BLOOM_SAMPLES);
	ImGui::SliderFloat("bloomThreshhold", &bloomThreshhold, 0, 2);
	ImGui::SliderFloat("sampleScale", &sampleScale, 0, 10);
}