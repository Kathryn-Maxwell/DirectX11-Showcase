#pragma once
// Renderer Class
// Used as a base class for ShaderRenderer, PathTraceRenderer, and PostProcessingRenderer
// A mostly virtual class with references to the scene meshes

#include <CubeMesh.h>
#include <RenderTexture.h>
#include <SphereMesh.h>
#include <vector>

class FPCamera;
class Timer;
class TextureManager;
class D3D;
class LightManager;

class DefaultShader;
class TerrainShader;
class WaveShader;
class TessellationPlaneMesh;

#pragma once
class Renderer
{
public:
	Renderer(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, Timer* _timer, FPCamera* _camera, DirectX::XMINT2 _screenSize);
	~Renderer();

	virtual RenderTexture* render() { return nullptr; }
	virtual void gui();
	static void SetGuiTheme();

protected:
	D3D* renderer = nullptr;
	LightManager* lightManager = nullptr;
	TextureManager* textureManager = nullptr;
	Timer* timer = nullptr;
	FPCamera* camera = nullptr;
	XMINT2 screenSize = XMINT2(0, 0);

	// Shaders
	DefaultShader* defaultShader = nullptr;
	TerrainShader* terrainShader = nullptr;
	WaveShader* waveShader = nullptr;

	// Meshes
	TessellationPlaneMesh* terrainMesh = nullptr;
	TessellationPlaneMesh* waterMesh = nullptr;
	std::vector<CubeMesh*> cubes;

	std::vector<SphereMesh*> spheres;
};