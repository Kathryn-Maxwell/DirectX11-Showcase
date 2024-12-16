// Main.cpp
#include "../DXFramework/System.h"
#include "App1.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	App1* app = new App1();

	// Create the system object.
	System* system = new System(app, 1200, 675, true, false);

	// Initialize and run the system object.
	system->run();

	// system deletes app, not sure why the framework takes ownership but doesn't use 'Move'. or why it needs to be passed an App and not generate it itself.
	delete system;
	system = nullptr;

	return 0;
}