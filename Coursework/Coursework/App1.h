#pragma once
// Application.h
#ifndef _APP1_H
#define _APP1_H

#include "DXF.h"

class ShaderRenderer;
class PathTraceRenderer;
class PostProcessingRenderer;
class LightManager;
class DefaultShader;

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE _hinstance, HWND _hwnd, int _screenWidth, int _screenHeight, Input* _in, bool _VSYNC, bool _FULL_SCREEN);

	bool frame();

protected:
	bool render();
	virtual void gui();

	LightManager* lightManager = nullptr;

	ShaderRenderer* shaderRenderer = nullptr;
	PathTraceRenderer* pathTraceRenderer = nullptr;
	PostProcessingRenderer* postProcessingRenderer = nullptr;

	DefaultShader* defaultShader = nullptr;
	OrthoMesh* orthoMesh = nullptr;

	XMINT2 screenSize = XMINT2(0, 0);
	bool pathTraceLighting = false;
};

#endif