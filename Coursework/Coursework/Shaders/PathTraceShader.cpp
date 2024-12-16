#include "PathTraceShader.h"

PathTraceShader::PathTraceShader(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, FPCamera* _camera, XMINT2 _screenSize, std::vector<SphereMesh*>* _spheres)
	: DefaultShader(_renderer, _hwnd, _lightManager, _textureManager, _camera)
{
	spheres = _spheres;
	screenSize = _screenSize;

	initShader();

#pragma region Spheres
	sphereData[0] = CreateSphere();
	sphereData[0].position = XMFLOAT3(25, -11, 40);
	sphereData[0].radius = 15;
	sphereData[0].material.colour = XMFLOAT4(0, 0.8f, 0.6f, 1);
	sphereData[0].material.smoothness = 0.1f;
	sphereData[0].material.specularColour = XMFLOAT4(1, 1, 1, 1);
	sphereData[0].material.specularPorbability = 0.02f;
	activeSpheres.push_back(&sphereData[0]);

	sphereData[1] = CreateSphere();
	sphereData[1].position = XMFLOAT3(25, 5, 40);
	sphereData[1].material.smoothness = 0.9f;
	sphereData[1].material.specularPorbability = 0.4f;

	sphereData[2] = CreateSphere();
	sphereData[2].position = XMFLOAT3(30, 5, 40);
	sphereData[2].material.colour = XMFLOAT4(0, 0.502f, 0.502f, 1);
	sphereData[2].material.smoothness = 0.4f;
	sphereData[2].material.specularPorbability = 0.15f;

	sphereData[3] = CreateSphere();
	sphereData[3].position = XMFLOAT3(20, 5, 40);
	sphereData[3].material.colour = XMFLOAT4(1, 0, 0, 1);

	sphereData[4] = CreateSphere();
	sphereData[4].position = XMFLOAT3(0, 15, 50);
	sphereData[4].radius = 20;
	sphereData[4].material.colour = XMFLOAT4(0, 0, 0, 1);
	sphereData[4].material.emissionColour = XMFLOAT4(0.8f, 0.8f, 0.8f, 1);
	sphereData[4].material.emissionStrenth = 2;
	sphereData[4].material.specularColour = XMFLOAT4(1, 1, 1, 1);

	sphereData[5] = CreateSphere();
	sphereData[5].position = XMFLOAT3(40, 15, 30);
	sphereData[5].radius = 10;
	sphereData[5].material.colour = XMFLOAT4(0, 0, 0, 1);
	sphereData[5].material.emissionColour = XMFLOAT4(0.8f, 0.8f, 0.8f, 1);
	sphereData[5].material.emissionStrenth = 5;
	sphereData[5].material.specularColour = XMFLOAT4(1, 1, 1, 1);

	for (size_t sphere = 0; sphere < 5; sphere++)
	{
		activeSpheres.push_back(&sphereData[sphere]);
	}
#pragma endregion
}

PathTraceShader::~PathTraceShader()
{
	for (int sphere = 0; sphere < activeSpheres.size(); sphere++)
	{
		delete activeSpheres[sphere];
		activeSpheres[sphere] = nullptr;
	}
	activeSpheres.clear();

	spheres = nullptr;

	delete cameraDataBuffer;
	cameraDataBuffer = nullptr;
	
	delete sphereDataBuffer;
	sphereDataBuffer = nullptr;
	
	delete rayTracingSettingsDataBuffer;
	rayTracingSettingsDataBuffer = nullptr;
	
	DefaultShader::~DefaultShader();
}

SphereData PathTraceShader::CreateSphere()
{
	SphereData sphere;
	sphere.position = XMFLOAT3(0, 0, 0);
	sphere.radius = 1;
	sphere.material = CreateRayTracingMaterial();
	return sphere;
}

RayTracingMaterial PathTraceShader::CreateRayTracingMaterial()
{
	RayTracingMaterial material;
	material.colour = XMFLOAT4(1, 1, 1, 1);
	material.emissionColour = XMFLOAT4(0, 0, 0, 1);
	material.specularColour = XMFLOAT4(1, 1, 1, 1);
	material.emissionStrenth = 0;
	material.smoothness = 0;
	material.specularPorbability = 0;
	material.padding = 0;
	return material;
}

void PathTraceShader::initShader()
{
	// Load (+ compile) shader files
	loadVertexShader(L"default_vs.cso");
	loadPixelShader(L"path-trace_ps.cso");

	createDefaultBuffers();

	D3D11_BUFFER_DESC cameraDataBufferDesc;
	cameraDataBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraDataBufferDesc.ByteWidth = sizeof(CameraDataBufferType);
	cameraDataBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraDataBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraDataBufferDesc.MiscFlags = 0;
	cameraDataBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&cameraDataBufferDesc, NULL, &cameraDataBuffer);

	D3D11_BUFFER_DESC sphereDataBufferDesc;
	sphereDataBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	sphereDataBufferDesc.ByteWidth = sizeof(MeshesBufferType);
	sphereDataBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sphereDataBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sphereDataBufferDesc.MiscFlags = 0;
	sphereDataBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&sphereDataBufferDesc, NULL, &sphereDataBuffer);

	D3D11_BUFFER_DESC rayTracingSettingsBufferDesc;
	rayTracingSettingsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	rayTracingSettingsBufferDesc.ByteWidth = sizeof(RayTracingSettingsBufferType);
	rayTracingSettingsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	rayTracingSettingsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	rayTracingSettingsBufferDesc.MiscFlags = 0;
	rayTracingSettingsBufferDesc.StructureByteStride = 0;
	rendererDevice->CreateBuffer(&rayTracingSettingsBufferDesc, NULL, &rayTracingSettingsDataBuffer);
}

void PathTraceShader::render(BaseMesh* _mesh, const Matrices _matrices, bool _multiPassRendering)
{
	deviceContext = renderer->getDeviceContext();

	setShaderParameters(_matrices, _multiPassRendering);
	_mesh->sendData(deviceContext);
	BaseShader::render(deviceContext, _mesh->getIndexCount());
}

void PathTraceShader::setShaderParameters(const Matrices _matrices, bool _multiPassRendering)
{
	deviceContext = renderer->getDeviceContext();

	mapMatrixBuffer(deviceContext, _matrices);

	D3D11_MAPPED_SUBRESOURCE mappedResource;

#pragma region Camera Data Buffer
	deviceContext->Map(cameraDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CameraDataBufferType* cameraDataPtr = (CameraDataBufferType*)mappedResource.pData;

	float aspect = (float)screenSize.x / (float)screenSize.y;
	float planeHeight = (float)(SCREEN_NEAR * tan(fov * 0.5 * Deg2Rad) * 2);
	float planeWidth = planeHeight * aspect;

	float pitch = (float)(camera->getRotation().x * Deg2Rad);
	float yaw = (float)(camera->getRotation().y * Deg2Rad);
	
	cameraForward.y = -sin(pitch);
	cameraForward.z = cos(pitch) * cos(yaw);

	cameraRight.x = cos(yaw);
	cameraRight.y = 0;
	cameraRight.z = -sin(yaw);

	cameraDataPtr->screenSize = screenSize;
	cameraDataPtr->planeSize = XMFLOAT2(planeWidth, planeHeight);

	cameraDataPtr->cameraWorldPosition = XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 1);
	
	cameraDataPtr->cameraForward = cameraForward;
	cameraDataPtr->nearClipPlane = SCREEN_NEAR;

	cameraDataPtr->cameraRight = cameraRight;
	cameraDataPtr->padding = 0.0f;

	deviceContext->Unmap(cameraDataBuffer, 0);
#pragma endregion

#pragma region Sphere Data Buffer
	deviceContext->Map(sphereDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MeshesBufferType* meshDataPtr = (MeshesBufferType*)mappedResource.pData;
	for (int sphere = 0; sphere < MAX_SPHERES; sphere++)
	{
		meshDataPtr->sphereData[sphere] = sphereData[sphere];
	}
	deviceContext->Unmap(sphereDataBuffer, 0);
#pragma endregion

#pragma region Ray Tracing Settings Buffer
	deviceContext->Map(rayTracingSettingsDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	RayTracingSettingsBufferType* rayTracingSettingsDataPtr = (RayTracingSettingsBufferType*)mappedResource.pData;
	rayTracingSettingsDataPtr->maxRayBounce = maxRayBounce;
	rayTracingSettingsDataPtr->raysPerPixel = raysPerPixel;
	if (_multiPassRendering) framesRendered++;
	rayTracingSettingsDataPtr->multiPassFramesRendered = framesRendered;
	rayTracingSettingsDataPtr->sphereCount = (int)activeSpheres.size();
	deviceContext->Unmap(rayTracingSettingsDataBuffer, 0);
#pragma endregion
	
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->PSSetConstantBuffers(1, 1, &cameraDataBuffer);
	deviceContext->PSSetConstantBuffers(2, 1, &sphereDataBuffer);
	deviceContext->PSSetConstantBuffers(3, 1, &rayTracingSettingsDataBuffer);
}

void PathTraceShader::gui()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::Text("Camera Forward: X %.2f, Y %.2f, Z %.2f", cameraForward.x, cameraForward.y, cameraForward.z);
		ImGui::Text("Camera Right: X %.2f, Y %.2f, Z %.2f", cameraRight.x, cameraRight.y, cameraRight.z);
		ImGui::SliderFloat("Camera FOV: %f", &fov, 30, 180, "%.0f");
	}

	if (ImGui::CollapsingHeader("RayTracing"))
	{
		ImGui::SliderInt("Max Ray Bounce", &maxRayBounce, 0, 30);
		ImGui::SliderInt("Rays Per Pixel", &raysPerPixel, 1, 30);
	}

	if (ImGui::CollapsingHeader("Spheres"))
	{
		for (size_t sphere = 0; sphere < activeSpheres.size(); sphere++)
		{
			if (ImGui::TreeNode((void*)(intptr_t)sphere, "Sphere: %d", sphere))
			{
				ImGui::SliderFloat3("Position", &sphereData[sphere].position.x, -50, 50);
				ImGui::SliderFloat("Radius", &sphereData[sphere].radius, 0.1f, 30);

				ImGui::Spacing();

				ImGui::ColorEdit4("Colour", &sphereData[sphere].material.colour.x, ImGuiColorEditFlags_NoInputs);

				ImGui::Spacing();

				ImGui::ColorEdit4("Emission colour", &sphereData[sphere].material.emissionColour.x, ImGuiColorEditFlags_NoInputs);
				ImGui::SliderFloat("Emission strength", &sphereData[sphere].material.emissionStrenth, 0, 10);

				ImGui::Spacing();

				ImGui::ColorEdit4("Specular colour", &sphereData[sphere].material.emissionColour.x, ImGuiColorEditFlags_NoInputs);
				ImGui::SliderFloat("Specular probability", &sphereData[sphere].material.specularPorbability, 0, 1);
				ImGui::SliderFloat("Smoothness", &sphereData[sphere].material.smoothness, 0, 1);

				ImGui::TreePop();
			}
		}

		if (activeSpheres.size() < MAX_SPHERES)
		{
			if (ImGui::Button("Add Sphere"))
			{
				for (int sphere = 0; sphere < MAX_SPHERES; sphere++)
				{
					if (std::find(activeSpheres.begin(), activeSpheres.end(), &sphereData[sphere]) == activeSpheres.end())
					{
						activeSpheres.push_back(&sphereData[sphere]);
						break;
					}
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Sphere"))
			{
				activeSpheres.erase(activeSpheres.begin() + activeSpheres.size() - 1);
			}
		}
	}
}