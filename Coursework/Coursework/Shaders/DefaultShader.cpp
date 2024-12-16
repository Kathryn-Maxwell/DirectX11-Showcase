#include "DefaultShader.h"
#include <TextureManager.h>
#include "../LightManager.h"

DefaultShader::DefaultShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera) 
	: BaseShader(_renderer->getDevice(), _hwnd)
{
	renderer = _renderer;

	lightManager = _lightManager;
	textureManager = _textureManager;
	camera = _camera;
	initShader();

	defaultShaderSettings.renderTexture = true;
	defaultShaderSettings.renderLighting = true;
	defaultShaderSettings.renderNormals = false;
}

DefaultShader::~DefaultShader()
{
	delete wrapSampler;
	wrapSampler = nullptr;

	delete clampSampler;
	clampSampler = nullptr;

	delete shaderSettingsBuffer;
	shaderSettingsBuffer = nullptr;

	delete matrixBuffer;
	matrixBuffer = nullptr;	

	delete lightBuffer;
	lightBuffer = nullptr;

	renderer = nullptr;
	lightManager = nullptr;
	textureManager = nullptr;	
	camera = nullptr;
	deviceContext = nullptr;
	
	//Release base shader components
	BaseShader::~BaseShader();
}

void DefaultShader::initShader()
{
	loadVertexShader(L"default_vs.cso");
	loadPixelShader(L"default_ps.cso");
	//loadGeometryShader(L"default_gs.cso");
	
	createDefaultBuffers();
}

void DefaultShader::createDefaultBuffers()
{
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture wrap sampler state description.
	D3D11_SAMPLER_DESC wrapSamplerDesc;
	wrapSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	wrapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	wrapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	wrapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	wrapSamplerDesc.MipLODBias = 0.0f;
	wrapSamplerDesc.MaxAnisotropy = 1;
	wrapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	wrapSamplerDesc.MinLOD = 0;
	wrapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	rendererDevice->CreateSamplerState(&wrapSamplerDesc, &wrapSampler);

	// Create a texture clamp sampler state description.
	D3D11_SAMPLER_DESC clampSamplerDesc;
	clampSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	clampSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampSamplerDesc.MipLODBias = 0.0f;
	clampSamplerDesc.MaxAnisotropy = 1;
	clampSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	clampSamplerDesc.MinLOD = 0;
	clampSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	rendererDevice->CreateSamplerState(&clampSamplerDesc, &clampSampler);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Setup settings buffer
	D3D11_BUFFER_DESC defaultShaderSettingsBufferDesc;
	defaultShaderSettingsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	defaultShaderSettingsBufferDesc.ByteWidth = sizeof(DefaultShaderSettingsBufferType);
	defaultShaderSettingsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	defaultShaderSettingsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	defaultShaderSettingsBufferDesc.MiscFlags = 0;
	defaultShaderSettingsBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&defaultShaderSettingsBufferDesc, NULL, &shaderSettingsBuffer);
}

void DefaultShader::renderMesh(BaseMesh* _mesh, Matrices _matrices, ID3D11ShaderResourceView* _texture)
{
	deviceContext = renderer->getDeviceContext();

	setShaderParameters(_mesh, _matrices, _texture);
	_mesh->sendData(deviceContext);
	BaseShader::render(deviceContext, _mesh->getIndexCount());
}

void DefaultShader::setShaderParameters(BaseMesh* _mesh, Matrices _matrices, ID3D11ShaderResourceView* _texture)
{
	deviceContext = renderer->getDeviceContext();

	ID3D11ShaderResourceView* texture = _texture == nullptr? textureManager->getTexture(L"default") : _texture;

	_matrices.worldMatrix *= _mesh->getTranslation();

	mapMatrixBuffer(deviceContext, _matrices);
	mapLightBuffer(deviceContext);
	mapDefaultShaderSettingsBuffer(deviceContext);

	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	deviceContext->GSSetConstantBuffers(0, 1, &matrixBuffer);

	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);
	deviceContext->PSSetConstantBuffers(1, 1, &shaderSettingsBuffer);
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &wrapSampler);
}

void DefaultShader::mapMatrixBuffer(ID3D11DeviceContext* _deviceContext, const Matrices& _matrices)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	_deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = XMMatrixTranspose(_matrices.worldMatrix);
	dataPtr->view = XMMatrixTranspose(_matrices.viewMatrix);
	dataPtr->projection = XMMatrixTranspose(_matrices.projectionMatrix);
	_deviceContext->Unmap(matrixBuffer, 0);
}

// Send light data to pixel shader
void DefaultShader::mapLightBuffer(ID3D11DeviceContext* _deviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	_deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	LightBufferType* lightPtr = (LightBufferType*)mappedResource.pData;
	for (int light = 0; light < MAX_LIGHTS; light++)
	{
		lightPtr->lightData[light] = lightManager->getLights()[light];
	}
	_deviceContext->Unmap(lightBuffer, 0);
}

void DefaultShader::mapDefaultShaderSettingsBuffer(ID3D11DeviceContext* _deviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	_deviceContext->Map(shaderSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	DefaultShaderSettingsBufferType* settingsPtr = (DefaultShaderSettingsBufferType*)mappedResource.pData;
	settingsPtr->renderTexture = defaultShaderSettings.renderTexture;
	settingsPtr->renderLighting = defaultShaderSettings.renderLighting;
	settingsPtr->renderNormals = defaultShaderSettings.renderNormals;
	settingsPtr->padding = 0;
	_deviceContext->Unmap(shaderSettingsBuffer, 0);
}