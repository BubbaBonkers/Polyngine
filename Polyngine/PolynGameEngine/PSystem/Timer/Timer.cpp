#include "Timer.h"

// Constructor.
Timer::Timer()
{
	start = std::chrono::high_resolution_clock::now();
	stop = std::chrono::high_resolution_clock::now();
}

// Get time elapsed in miliseconds since the last tick.  
double Timer::GetElapsedMiliseconds()
{
	if (bActivated)
	{
		// Get time since last Restart.
		auto Elapsed = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start);
		return Elapsed.count();
	}
	else
	{
		// Get time since stopped.
		auto Elapsed = std::chrono::duration<double, std::milli>(stop - start);
		return Elapsed.count();
	}
}

// Restart the Timer object's count.
void Timer::Restart()
{
	// Set the start time to the current time.
	bActivated = true;
	start = std::chrono::high_resolution_clock::now();
}

// Start the Timer object's count.
bool Timer::Start()
{
	if (bActivated)
	{
		// Already started.
		return false;
	}
	else
	{
		// Set the start time to now, return true as true.
		start = std::chrono::high_resolution_clock::now();
		bActivated = true;
		return true;
	}
}

// Stop the Timer object's count.
bool Timer::Stop()
{
	if (!bActivated)
	{
		// Timer already stopped.
		return false;
	}
	else
	{
		// Set the stop time to now and return true.
		stop = std::chrono::high_resolution_clock::now();
		bActivated = false;
		return true;
	}
}
