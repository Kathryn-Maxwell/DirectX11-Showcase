#include "TerrainShader.h"
#include "../LightManager.h"
#include "../TessellationPlaneMesh.h"
#include "TextureManager.h"

TerrainShader::TerrainShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera)
	: DefaultShader(_renderer, _hwnd, _lightManager, _textureManager, _camera)
{
	// GUI variables
	tessellationTarget = XMFLOAT3(46.0f, 3.5f, 38.5f);
	tessellationStrength = 15.0f;
	heightMapMaxHeight = 3.5f;

	initShader();
}

TerrainShader::~TerrainShader()
{
	delete dynamicTessellationBuffer;
	dynamicTessellationBuffer = nullptr;

	delete heightMapBuffer;
	heightMapBuffer = nullptr;
	
	//Release base shader components
	BaseShader::~BaseShader();
}

void TerrainShader::initShader()
{
	// Load (+ compile) shader files
	loadVertexShader(L"terrain_vs.cso");
	loadHullShader(L"terrain_hs.cso");
	loadDomainShader(L"terrain_ds.cso");
	loadPixelShader(L"terrain_ps.cso");
	
	createDefaultBuffers();

	D3D11_BUFFER_DESC dynamicTessellationBufferDesc;
	dynamicTessellationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	dynamicTessellationBufferDesc.ByteWidth = sizeof(DynamicTessellationBufferType);
	dynamicTessellationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dynamicTessellationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dynamicTessellationBufferDesc.MiscFlags = 0;
	dynamicTessellationBufferDesc.StructureByteStride = 0;

	rendererDevice->CreateBuffer(&dynamicTessellationBufferDesc, NULL, &dynamicTessellationBuffer);
	D3D11_BUFFER_DESC heightMapBufferDesc;
	heightMapBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	heightMapBufferDesc.ByteWidth = sizeof(HeightMapBufferType);
	heightMapBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	heightMapBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	heightMapBufferDesc.MiscFlags = 0;
	heightMapBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&heightMapBufferDesc, NULL, &heightMapBuffer);
}

void TerrainShader::renderMesh(TessellationPlaneMesh* _sandMesh, Matrices _matrices)
{
	deviceContext = renderer->getDeviceContext();

	setShaderParameters(_sandMesh, _matrices);
	_sandMesh->sendData(deviceContext);
	BaseShader::render(deviceContext, _sandMesh->getIndexCount());
}

void TerrainShader::setShaderParameters(TessellationPlaneMesh* _sandMesh, Matrices _matrices)
{
	deviceContext = renderer->getDeviceContext();

	_matrices.worldMatrix *= _sandMesh->getTranslation();

	mapMatrixBuffer(deviceContext, _matrices);
	mapLightBuffer(deviceContext);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ID3D11ShaderResourceView* heightMap = textureManager->getTexture(L"height");
	ID3D11ShaderResourceView* albedo = textureManager->getTexture(L"sand");
	ID3D11ShaderResourceView* normalMap = textureManager->getTexture(L"sand_normal");

	// Tessellation buffer
	deviceContext->Map(dynamicTessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	DynamicTessellationBufferType* dynamicTessellationPtr = (DynamicTessellationBufferType*)mappedResource.pData;
	dynamicTessellationPtr->tessellationTarget = tessellationTarget;
	dynamicTessellationPtr->tessellationStrength = tessellationStrength;
	deviceContext->Unmap(dynamicTessellationBuffer, 0);

	// Height map buffer
	deviceContext->Map(heightMapBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	HeightMapBufferType* heightMapPtr = (HeightMapBufferType*)mappedResource.pData;
	heightMapPtr->maxHeight = heightMapMaxHeight;
	heightMapPtr->meshSize = XMINT2(50,50);
	heightMapPtr->textureScale = 10.0f;
	deviceContext->Unmap(heightMapBuffer, 0);

	deviceContext->HSSetConstantBuffers(1, 1, &dynamicTessellationBuffer);
	deviceContext->HSSetConstantBuffers(0, 1, &matrixBuffer);

	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->DSSetConstantBuffers(1, 1, &heightMapBuffer);
	deviceContext->DSSetShaderResources(0, 1, &heightMap);
	deviceContext->DSSetSamplers(0, 1, &clampSampler);

	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);
	deviceContext->PSSetConstantBuffers(1, 1, &heightMapBuffer);
	deviceContext->PSSetShaderResources(0, 1, &heightMap);
	deviceContext->PSSetShaderResources(1, 1, &albedo);
	deviceContext->PSSetShaderResources(2, 1, &normalMap);
	deviceContext->PSSetSamplers(0, 1, &clampSampler);
	deviceContext->PSSetSamplers(1, 1, &wrapSampler);
}

void TerrainShader::gui()
{
	if (ImGui::CollapsingHeader("Tessellation"))
	{
		ImGui::DragFloat3("Tess Target", &tessellationTarget.x, 1.0f, -10, 100);
		ImGui::SliderFloat("Tess Strength", &tessellationStrength, 1, 20);
		ImGui::SliderFloat("Island Height", &heightMapMaxHeight, 0, 20);
	}
}