/**
* \class Timer
*
* \brief Calculates frame/delta time and FPS
*
* \author Paul Robertson
*/

// timer class
// Calculates frame/delta time and FPS.

#ifndef _TIMER_H_
#define _TIMER_H_

#include <windows.h>

class Timer
{
public:
	Timer();
	~Timer();

	void frame();	///< Update delta time, called every frame

	float getDeltaTime();		///< Get delta time
	float getApplicationTime();	///< Get application time
	float getFPS();		///< Get FPS (for display)

private:
	INT64 frequency;
	float ticksPerS;
	INT64 lastFrameTime;
	float frameTime;
	float fps;
	float frames;
	float elapsedTime;
	float applicationTime;
};

#endif