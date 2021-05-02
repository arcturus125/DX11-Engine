//--------------------------------------------------------------------------------------
// Timer class - works like a stopwatch
//--------------------------------------------------------------------------------------

#ifndef _TIMER_H_INCLUDED_
#define _TIMER_H_INCLUDED_

#include "Windows.h"

class Timer
{
public:

	// Constructor //

	__declspec(dllexport) Timer();

	
	// Timer control //

	// Start the timer running
	__declspec(dllexport) void Start();

	// Stop the timer running
	void Stop();

	// Reset the timer to zero
	void Reset();


	// Timing //

	// Get frequency of the timer being used (in counts per second)
	float GetFrequency();

	// Get time passed (seconds) since since timer was started or last reset
	float GetTime();

	// Get time passed (seconds) since last call to this function. If this is the first call, then
	// the time since timer was started or the last reset is returned
	__declspec(dllexport) float GetLapTime();


private:
	// Is the timer running
	bool mRunning;


	// Using high resolution timer and if so its frequency
	bool          mHighRes;
	LARGE_INTEGER mHighResFreq;

	// Start time and last lap start time of high-resolution timer
	LARGE_INTEGER mHighResStart;
	LARGE_INTEGER mHighResLap;

	// Time when high-resolution timer was stopped (if it has been)
	LARGE_INTEGER mHighResStop;


	// Start time and last lap start time of low-resolution timer
	DWORD mLowResStart;
	DWORD mLowResLap;

	// Time when low-resolution timer was stopped (if it has been)
	DWORD mLowResStop;
};


#endif //_TIMER_H_INCLUDED_
