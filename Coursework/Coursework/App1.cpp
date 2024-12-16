#include "App1.h"

#include "Renderers/ShaderRenderer.h"
#include "Renderers/PathTraceRenderer.h"
#include "Renderers/PostProcessingRenderer.h"

#include "LightManager.h"
#include "Shaders/DefaultShader.h"

App1::App1()
{

}

void App1::init(HINSTANCE _hinstance, HWND _hwnd, int _screenWidth, int _screenHeight, Input* _in, bool _VSYNC, bool _FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(_hinstance, _hwnd, _screenWidth, _screenHeight, _in, _VSYNC, _FULL_SCREEN);

	lightManager = new LightManager(renderer);

	// Load resources
	textureMgr->loadTexture(L"checker_board", L"res/checkerboard.jpg");
	textureMgr->loadTexture(L"default", L"res/default.jpg");
	textureMgr->loadTexture(L"height", L"res/CoastHeightMap_06.jpg");
	textureMgr->loadTexture(L"sand", L"res/sand/sand_albedo.jpg");
	textureMgr->loadTexture(L"sand_normal", L"res/sand/sand_normal.jpg");

	// Camera
	camera->setPosition(12.5f, 5.5f, 55.0f);
	camera->setRotation(20, 150, 0);

	screenSize = XMINT2(_screenWidth, _screenHeight);

	shaderRenderer = new ShaderRenderer(renderer, _hwnd, lightManager, textureMgr, timer, camera, screenSize);
	pathTraceRenderer = new PathTraceRenderer(renderer, _hwnd, lightManager, textureMgr, timer, camera, screenSize);
	postProcessingRenderer = new PostProcessingRenderer(renderer, _hwnd, lightManager, textureMgr, timer, camera, screenSize);

	defaultShader = new DefaultShader(renderer, _hwnd, lightManager, textureMgr, camera);
	defaultShader->getDefaultShaderSettings()->renderLighting = false;

	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenSize.x, screenSize.y, 0, 0);
}

App1::~App1()
{
	delete orthoMesh;
	orthoMesh = nullptr;

	delete defaultShader;
	defaultShader = nullptr;

	delete postProcessingRenderer;
	postProcessingRenderer = nullptr;

	delete pathTraceRenderer;
	pathTraceRenderer = nullptr;

	delete shaderRenderer;
	shaderRenderer = nullptr;

	delete lightManager;
	lightManager = nullptr;

	// Run base application deconstructor
	BaseApplication::~BaseApplication();
}

bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result) return false;	
	
	// Render the graphics.
	result = render();
	if (!result) return false;	

	return true;
}

bool App1::render()
{
	if (wireframeToggle)
	{
		shaderRenderer->renderWireFrame();
		gui();
		renderer->endScene();
		return true;
	}

	// Select renderer and render the frame
	RenderTexture* currentFrame = pathTraceLighting ? pathTraceRenderer->render() : shaderRenderer->render();
	RenderTexture* postProcessedFrame = postProcessingRenderer->render(currentFrame);

	// Set render target to buffer (renderers render to a RenderTexture)
	renderer->setBackBufferRenderTarget();

	camera->update();

	// Clear the scene. (default grey colour)
	renderer->beginScene(0.2f, 0.2f, 0.2f, 1.0f);

	ID3D11DeviceContext* deviceContext = renderer->getDeviceContext();
	Matrices matrices = Matrices(renderer->getWorldMatrix(), camera->getOrthoViewMatrix(), renderer->getOrthoMatrix());

	// Render ortho mesh
	renderer->setZBuffer(false);
	defaultShader->renderMesh(orthoMesh, matrices, postProcessedFrame->getShaderResourceView());
	renderer->setZBuffer(true);

	// Render UI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	ID3D11DeviceContext* deviceContext = renderer->getDeviceContext();
	deviceContext->GSSetShader(NULL, NULL, 0);
	deviceContext->HSSetShader(NULL, NULL, 0);
	deviceContext->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::PushItemWidth(200);
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("Ray Tracing", &pathTraceLighting);
	ImGui::Separator();

	// Select renderer and render gui
	pathTraceLighting ? pathTraceRenderer->gui() : shaderRenderer->gui();
	postProcessingRenderer->gui();

	ImGui::PopItemWidth();

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}