#pragma once
// Default Shader
//
//

#include "ShaderData.h"

class LightManager;
class TextureManager;

class DefaultShader : public BaseShader
{
public:
	struct DefaultShaderSettings
	{
		bool renderTexture;
		bool renderLighting;
		bool renderNormals;
	};

	struct DefaultShaderSettingsBufferType
	{
		int renderTexture;
		int renderLighting;
		int renderNormals;
		int padding;
	};

	struct LightBufferType
	{
		LightData lightData[MAX_LIGHTS];
	};

public:
	DefaultShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera);
	~DefaultShader();

	void renderMesh(BaseMesh* _mesh, Matrices _matrices, ID3D11ShaderResourceView* _texture);
	
	virtual void gui() {}

	DefaultShaderSettings* getDefaultShaderSettings() { return &defaultShaderSettings; }

protected:
	virtual void initShader();
	void createDefaultBuffers();

	void mapMatrixBuffer(ID3D11DeviceContext* _deviceContext, const Matrices& _matrices);
	void mapLightBuffer(ID3D11DeviceContext* _deviceContext);
	void mapDefaultShaderSettingsBuffer(ID3D11DeviceContext* _deviceContext);

	void setShaderParameters(BaseMesh* _mesh, Matrices _matrices, ID3D11ShaderResourceView* _texture);

protected:
	D3D* renderer = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	LightManager* lightManager = nullptr;
	TextureManager* textureManager = nullptr;	
	FPCamera* camera = nullptr;

	ID3D11Buffer* matrixBuffer = nullptr;
	ID3D11SamplerState* wrapSampler = nullptr;
	ID3D11SamplerState* clampSampler = nullptr;
	ID3D11Buffer* lightBuffer = nullptr;
	ID3D11Buffer* shaderSettingsBuffer = nullptr;

	DefaultShaderSettings defaultShaderSettings;
};
