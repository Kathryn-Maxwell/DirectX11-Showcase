// Timer object.
// Calculate delta/frame time and FPS.
#include "timer.h"

// Initialise timer. Check for high performance timers.
Timer::Timer()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	if (frequency == 0)
	{
		MessageBox(NULL, L"No support for high performance timer", L"ERROR", MB_OK);
	}

	// Find out how many times the frequency counter ticks every second.
	ticksPerS = (float)(frequency);

	QueryPerformanceCounter((LARGE_INTEGER*)&lastFrameTime);

	applicationTime = 0.f;
	elapsedTime = 0.f;
	frames = 0.f;
	fps = 0.f;
}


Timer::~Timer()
{
}

// Once per frame calculate delta timer and update FPS calculation.
void Timer::frame()
{
	INT64 currentTime;
	float timeDifference;

	// Query the current time.
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	timeDifference = (float)(currentTime - lastFrameTime);
	frameTime = timeDifference / ticksPerS;

	// Calc FPS
	frames += 1.f;
	elapsedTime += frameTime;
	applicationTime += frameTime;
	if (elapsedTime > 1.0f)
	{
		fps = frames / elapsedTime;
		frames = 0.0f;
		elapsedTime = 0.0f;
	}

	// Restart the timer.
	lastFrameTime = currentTime;

	return;
}

float Timer::getDeltaTime()
{
	return frameTime;
}

float Timer::getApplicationTime()
{
	return applicationTime;
}

float Timer::getFPS()
{
	return fps;
}