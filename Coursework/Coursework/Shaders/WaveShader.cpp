#include "WaveShader.h"
#include "../LightManager.h"
#include "../TessellationPlaneMesh.h"
#include "TextureManager.h"

WaveShader::WaveShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera, Timer* _timer)
	: DefaultShader(_renderer, _hwnd, _lightManager, _textureManager, _camera)
{
	timer = _timer;

	waveData[0].amplitude = 0.297f;

	waveData[1].direction = XMFLOAT3(0.238f, -0.25f, -0.25f);
	waveData[1].amplitude = 0.368f;

	waveData[2].direction = XMFLOAT3(-0.95f, -0.1f, -0.1f);
	waveData[2].amplitude = 0.142f;

	waveData[3].direction = XMFLOAT3(-0.38f, 1.0f, 1.0f);
	waveData[3].timeScale = 0.42f;
	waveData[3].amplitude = 0.082f;

	initShader();
}

WaveShader::~WaveShader()
{
	timer = nullptr;
	
	delete waveDataBuffer;
	waveDataBuffer = nullptr;
	
	delete tessellationBuffer;
	tessellationBuffer = nullptr;
	
	BaseShader::~BaseShader();
}

void WaveShader::initShader()
{
	// Load (+ compile) shader files
	loadVertexShader(L"wave_vs.cso");
	loadHullShader(L"wave_hs.cso");
	loadDomainShader(L"wave_ds.cso");
	loadPixelShader(L"wave_ps.cso");
	//loadGeometryShader(L"default_gs.cso");

	createDefaultBuffers();

	D3D11_BUFFER_DESC waveDataBufferDesc;
	waveDataBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waveDataBufferDesc.ByteWidth = sizeof(WaveDataBufferType);
	waveDataBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waveDataBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waveDataBufferDesc.MiscFlags = 0;
	waveDataBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&waveDataBufferDesc, NULL, &waveDataBuffer);

	D3D11_BUFFER_DESC tessellationBufferDesc;
	tessellationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessellationBufferDesc.ByteWidth = sizeof(WaveDataBufferType);
	tessellationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessellationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessellationBufferDesc.MiscFlags = 0;
	tessellationBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&tessellationBufferDesc, NULL, &tessellationBuffer);
}

void WaveShader::renderMesh(TessellationPlaneMesh* _waterMesh, Matrices _matrices)
{
	deviceContext = renderer->getDeviceContext();

	setShaderParameters(_waterMesh, _matrices);
	_waterMesh->sendData(deviceContext);
	BaseShader::render(deviceContext, _waterMesh->getIndexCount());
}

void WaveShader::setShaderParameters(TessellationPlaneMesh* _waterMesh, Matrices _matrices)
{
	deviceContext = renderer->getDeviceContext();

	_matrices.worldMatrix *= _waterMesh->getTranslation();

	mapMatrixBuffer(deviceContext, _matrices);
	mapLightBuffer(deviceContext);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	//ID3D11ShaderResourceView* heightMap = textureManager->getTexture(L"height");

	// WaveData buffer
	deviceContext->Map(waveDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WaveDataBufferType* waterDataPtr = (WaveDataBufferType*)mappedResource.pData;
	float maxHeight = 0;
	for (int wave = 0; wave < NUM_WAVES; wave++)
	{
		waterDataPtr->waveData[wave] = waveData[wave];
		waterDataPtr->waveData[wave].direction.z = waveData[wave].direction.y; // convert float2 input to vector3
		waterDataPtr->waveData[wave].time = timer->getApplicationTime();
		maxHeight += waveData[wave].amplitude;
	}
	deviceContext->Unmap(waveDataBuffer, 0);

	// Tessellation buffer
	deviceContext->Map(tessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WaveTessellationBufferType* tessellationPtr = (WaveTessellationBufferType*)mappedResource.pData;
	tessellationPtr->maxTessellation = tessellationRange.y;
	tessellationPtr->minTessellation = tessellationRange.x;
	tessellationPtr->maxHeight = maxHeight;
	tessellationPtr->minHeight = -maxHeight;
	deviceContext->Unmap(tessellationBuffer, 0);

	deviceContext->HSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->HSSetConstantBuffers(1, 1, &waveDataBuffer);
	deviceContext->HSSetConstantBuffers(2, 1, &tessellationBuffer);

	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->DSSetConstantBuffers(1, 1, &waveDataBuffer);

	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);
	deviceContext->PSSetConstantBuffers(1, 1, &waveDataBuffer);
}

void WaveShader::gui()
{
	if (ImGui::CollapsingHeader("Waves"))
	{
		for (int wave = 0; wave < NUM_WAVES; wave++)
		{
			if (ImGui::TreeNode((void*)(intptr_t)wave, "Wave: %d", wave+1))
			{
				ImGui::SliderFloat2("Direction", &waveData[wave].direction.x, -1, 1, "%.2f", 0.5f);
				ImGui::SliderFloat("Phase", &waveData[wave].phase, 0, 10);
				ImGui::SliderFloat("Time Scale", &waveData[wave].timeScale, 0, 3);
				ImGui::SliderFloat("Gravity", &waveData[wave].gravity, 0, 10);
				ImGui::SliderFloat("Depth", &waveData[wave].depth, 1, 30);
				ImGui::SliderFloat("Amplitude", &waveData[wave].amplitude, 0, 1.2f);

				ImGui::TreePop();
			}
		}

		ImGui::Separator();
		ImGui::DragIntRange2("Tessellation Range", &tessellationRange.x, &tessellationRange.y, 0.1f, 1, 10);
	}
}