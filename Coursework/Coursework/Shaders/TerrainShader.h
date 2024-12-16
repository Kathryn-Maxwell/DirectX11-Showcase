#pragma once
// Terrain shader
// Dynamically tessellates a vertex manipulated plane based on a height map

#include "DefaultShader.h"

class TessellationPlaneMesh;

class TerrainShader : public DefaultShader
{
	struct DynamicTessellationBufferType {
		XMFLOAT3 tessellationTarget = XMFLOAT3(0.0f, 0.0f, 0.0f);
		float tessellationStrength = 5.0f;
	};

	struct HeightMapBufferType {
		float maxHeight = 10;
		XMINT2 meshSize = XMINT2(50, 50);
		float textureScale = 25.0f;
	};

public:

	TerrainShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera);
	~TerrainShader();

	void renderMesh(TessellationPlaneMesh* _sandMesh, Matrices _matrices);

	void gui() override;

private:
	void initShader() override;
	void setShaderParameters(TessellationPlaneMesh* _sandMesh, Matrices _matrices);

	ID3D11Buffer* heightMapBuffer = nullptr;
	ID3D11Buffer* dynamicTessellationBuffer = nullptr;

	// GUI variables
	XMFLOAT3 tessellationTarget = XMFLOAT3(5.5f, 0.0f, 5.5f);;
	float tessellationStrength = 5.0f;
	float heightMapMaxHeight = 10.0f;
};