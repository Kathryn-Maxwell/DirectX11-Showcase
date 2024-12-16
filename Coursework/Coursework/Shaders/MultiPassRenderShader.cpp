#include "MultiPassRenderShader.h"

MultiPassRenderShader::MultiPassRenderShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera)
	: DefaultShader(_renderer, _hwnd, _lightManager, _textureManager, _camera)
{
	initShader();
}

MultiPassRenderShader::~MultiPassRenderShader()
{
	delete multiPassRenderSettingsBuffer;
	multiPassRenderSettingsBuffer = nullptr;
	
	DefaultShader::~DefaultShader();
}

void MultiPassRenderShader::initShader()
{
	// Load (+ compile) shader files
	loadVertexShader(L"default_vs.cso");
	loadPixelShader(L"multi-pass_ps.cso");

	createDefaultBuffers();

	D3D11_BUFFER_DESC multiPassRenderSettingsBufferDesc;
	multiPassRenderSettingsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	multiPassRenderSettingsBufferDesc.ByteWidth = sizeof(MultiPassRenderSettingsBufferType);
	multiPassRenderSettingsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	multiPassRenderSettingsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	multiPassRenderSettingsBufferDesc.MiscFlags = 0;
	multiPassRenderSettingsBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&multiPassRenderSettingsBufferDesc, nullptr, &multiPassRenderSettingsBuffer);
}

void MultiPassRenderShader::render(BaseMesh* _mesh, const Matrices _matrices, RenderTexture* _currentFrame, RenderTexture* _lastFrame)
{
	deviceContext = renderer->getDeviceContext();

	setShaderParameters(_matrices, _currentFrame, _lastFrame);
	_mesh->sendData(deviceContext);
	BaseShader::render(deviceContext, _mesh->getIndexCount());
}

void MultiPassRenderShader::setShaderParameters(const Matrices _matrices, RenderTexture* _currentFrame, RenderTexture* _lastFrame)
{
	deviceContext = renderer->getDeviceContext();
	mapMatrixBuffer(deviceContext, _matrices);

	ID3D11ShaderResourceView* currentFrameTexture = _currentFrame->getShaderResourceView();
	ID3D11ShaderResourceView* lastFrameTexture = _lastFrame->getShaderResourceView();

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(multiPassRenderSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MultiPassRenderSettingsBufferType* multiPassRenderSettingsDataPtr = (MultiPassRenderSettingsBufferType*)mappedResource.pData;
	multiPassRenderSettingsDataPtr->multiPassFramesRendered = (float)multiPassFramesRendered;
	multiPassRenderSettingsDataPtr->padding = XMFLOAT3(1, 1, 1);
	deviceContext->Unmap(multiPassRenderSettingsBuffer, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &multiPassRenderSettingsBuffer);
	deviceContext->PSSetShaderResources(0, 1, &currentFrameTexture);
	deviceContext->PSSetShaderResources(1, 1, &lastFrameTexture);
	deviceContext->PSSetSamplers(0, 1, &wrapSampler);
	
	multiPassFramesRendered++;	
}

void MultiPassRenderShader::resetMultiPassRendering()
{
	multiPassFramesRendered = 0;
}

void MultiPassRenderShader::gui()
{
	if (ImGui::CollapsingHeader("Multi-Pass Rendering"))
	{
		ImGui::Checkbox("Enable Multi-Pass Rendering", &multiPassRendering);
		ImGui::Text("Multi-Pass Frames Rendered: %i", multiPassFramesRendered);
	}
}