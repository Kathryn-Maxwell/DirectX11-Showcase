#include "Renderer.h"

#include "../Shaders/DefaultShader.h"
#include "../LightManager.h"
#include "../TessellationPlaneMesh.h"

Renderer::Renderer(D3D* _renderer, HWND _hwnd, LightManager* _lightManager, TextureManager* _textureManager, Timer* _timer, FPCamera* _camera, XMINT2 _screenSize)
{
	renderer = _renderer;
	lightManager = _lightManager;
	textureManager = _textureManager;
	timer = _timer;
	camera = _camera;
	screenSize = _screenSize;

	// shaders
	defaultShader = new DefaultShader(renderer, _hwnd, lightManager, textureManager, camera);

	// Meshes
	terrainMesh = new TessellationPlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), XMINT2(100, 100));
	terrainMesh->setPosition(0, 0, -50.0f);

	waterMesh = new TessellationPlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), XMINT2(100, 100));
	waterMesh->setPosition(-60.0f, 1.2f, -50.0f);

	for (int cube = 0; cube < 5; cube++)
	{
		CubeMesh* mesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext(), 1);

		mesh->setPosition(XMFLOAT3(10.0f, 5.0f, cube * 10.0f));
		cubes.push_back(mesh);
	}

	SphereMesh* sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 8);
	sphere->setPosition(XMFLOAT3(20.0f, 5.0f, 40.0f));
	spheres.push_back(sphere);

	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 8);
	sphere->setPosition(XMFLOAT3(25.0f, 5.0f, 40.0f));
	spheres.push_back(sphere);

	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 8);
	sphere->setPosition(XMFLOAT3(30.0f, 5.0f, 40.0f));
	spheres.push_back(sphere);

	SetGuiTheme();
}

Renderer::~Renderer()
{
	renderer = nullptr;
	lightManager = nullptr;
	textureManager = nullptr;
	timer = nullptr;
	camera = nullptr;
	
	for (int Sphere = 0; Sphere < spheres.size(); Sphere++)
	{
		delete spheres[Sphere];
		spheres[Sphere] = nullptr;
	}
	spheres.clear();

	for (int cube = 0; cube < cubes.size(); cube++)
	{
		delete cubes[cube];
		cubes[cube] = nullptr;
	}
	cubes.clear();

	delete waterMesh;
	waterMesh = nullptr;

	delete terrainMesh;
	terrainMesh = nullptr;

	delete defaultShader;
	defaultShader = nullptr;
}

void Renderer::gui()
{
	lightManager->gui();
}

void Renderer::SetGuiTheme() {
	ImGuiStyle* style = &ImGui::GetStyle();

	style->Colors[ImGuiCol_TitleBg] = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_TitleBgActive] = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_CheckMark] = ImColor(0.9f, 0.9f, 0.9f, 1.0f);
	style->Colors[ImGuiCol_ResizeGrip] = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImColor(0.6f, 0.6f, 0.6f, 1.0f);
	style->Colors[ImGuiCol_Button] = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(0.6f, 0.6f, 0.6f, 1.0f);
	style->Colors[ImGuiCol_FrameBg] = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	style->Colors[ImGuiCol_FrameBgActive] = ImColor(0.6f, 0.6f, 0.6f, 1.0f);
	style->Colors[ImGuiCol_SliderGrab] = ImColor(0.6f, 0.6f, 0.6f, 1.0f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	style->Colors[ImGuiCol_Header] = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_HeaderActive] = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_HeaderHovered] = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
}