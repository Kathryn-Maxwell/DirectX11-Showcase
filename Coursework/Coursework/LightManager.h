#pragma once

#include "Shaders/ShaderData.h"

class DefaultShader;

class LightManager
{
public:
	LightManager(D3D* _renderer);
	~LightManager();

	void render(ID3D11DeviceContext* _deviceContext, DefaultShader* _defaultShader, Matrices _matrices) const;
	void gui();

	LightData* getLights() { return lights; }

private:
	enum class LightType { OFF, DIRECTIONAL, POINT, SPOT };

	LightData lights[MAX_LIGHTS];
	std::vector<LightData*> activeLights;
	std::vector<SphereMesh*> lightBoxes;

	bool renderLightboxes = true;

	// GUI
	const char* lightType[4]{ "OFF", "Directional", "Point", "Spot" };
};