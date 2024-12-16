#pragma once

#include "DXF.h"
#define MAX_LIGHTS 10
#define MAX_SPHERES 10
#define MAX_BLOOM_SAMPLES 7
#define PI 3.14159265359
#define Deg2Rad (PI*2) / 360.0f

struct Matrices {
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

	Matrices(XMMATRIX _worldMatrix, XMMATRIX _viewMatrix, XMMATRIX _projectionMatrix) {
		worldMatrix = _worldMatrix;
		viewMatrix = _viewMatrix;
		projectionMatrix = _projectionMatrix;
	}
};

struct Attenuation {
	float constant = 0.5f;
	float linear = 0.025f;
	float exponential = 0.05f;
	float padding = 0.0f;
};

struct LightData {
	int type = 0;
	XMINT3 typePadding = XMINT3(0,0,0);

	XMFLOAT4 position = XMFLOAT4(0.0f,0.0f,0.0f,0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	XMFLOAT4 ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	Attenuation attenuation;
};

struct RayTracingMaterial {
	XMFLOAT4 colour;
	XMFLOAT4 emissionColour;
	XMFLOAT4 specularColour;

	float emissionStrenth;
	float smoothness;
	float specularPorbability;
	float padding;
};

struct SphereData {
	XMFLOAT3 position;
	float radius;

	RayTracingMaterial material;
};