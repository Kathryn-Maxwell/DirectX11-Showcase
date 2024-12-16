#pragma once
// Path Trace Shader
//
//

#include "DefaultShader.h"

class LightManager;
class TextureManager;

class PathTraceShader : public DefaultShader
{
	struct CameraDataBufferType {
		XMINT2 screenSize;
		XMFLOAT2 planeSize;

		XMFLOAT4 cameraWorldPosition;

		XMFLOAT3 cameraForward;
		float nearClipPlane;

		XMFLOAT3 cameraRight;
		float padding;
	};

	struct MeshesBufferType {
		SphereData sphereData[MAX_SPHERES];
	};

	struct RayTracingSettingsBufferType {
		int maxRayBounce;
		int raysPerPixel;
		int multiPassFramesRendered;
		int sphereCount;
	};

public:
	PathTraceShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera, XMINT2 _screenSize, std::vector<SphereMesh*>* _spheres);
	~PathTraceShader();

	void render(BaseMesh* _mesh, const Matrices _matrices, bool _multiPassRendering);

	void gui() override;

private:
	void initShader() override;
	void setShaderParameters(const Matrices _matrices, bool _multiPassRendering);

	SphereData CreateSphere();
	RayTracingMaterial CreateRayTracingMaterial();

	std::vector<SphereMesh*>* spheres = nullptr;

	XMINT2 screenSize = XMINT2(0, 0);

	ID3D11Buffer* cameraDataBuffer = nullptr;
	ID3D11Buffer* sphereDataBuffer = nullptr;
	ID3D11Buffer* rayTracingSettingsDataBuffer = nullptr;

	XMFLOAT3 cameraForward;
	XMFLOAT3 cameraRight;
	float fov = 45;

	int maxRayBounce = 7;
	int raysPerPixel = 7;
	int framesRendered = 0;

	SphereData sphereData[MAX_SPHERES];
	std::vector<SphereData*> activeSpheres;

	int sphereCount = 6;
};