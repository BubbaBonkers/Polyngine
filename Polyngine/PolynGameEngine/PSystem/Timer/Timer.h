#include <chrono>

#pragma once

class Timer
{
public:
	Timer();														// Constructor.
	double GetElapsedMiliseconds();									// Get time elapsed in miliseconds since the last tick. 
	void Restart();													// Restart the Timer object's count.
	bool Start();													// Start the Timer object's count.
	bool Stop();													// Stop the Timer object's count.

private:
	bool bActivated = false;										// Whether to Timer is currently running.

	std::chrono::time_point < std::chrono::steady_clock> start;		// Start time point.
	std::chrono::time_point < std::chrono::steady_clock> stop;		// Stop time point.
};

