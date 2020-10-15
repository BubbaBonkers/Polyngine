#pragma once

// Use with Update() function, do not run in loops.
class PStopwatch
{
private:
	// The amount of time the stopwatch started with. If you create and set a 15 second stopwatch, the StartTime will be 15 seconds.
	float StartTime;
	float Expiration;
	bool bRecycle;				// Should this timer restart after timing out?
	unsigned int Cycles;		// How many times this stopwatch has recycled.

	bool bDisable;

public:
	PStopwatch();
	PStopwatch(float Time, bool bShouldRecycle = false);

	// Called once per frame.
	void Update(float DeltaTime);

	// Start the timer if created with a default constructor.
	void Start(float Time, bool bShouldRecycle = false);

	// Returns the amount of time in seconds that has passed since this stopwatch was started.
	float GetTimeAlive();

	// This will immediately lap the timer back to the StartTime. Optionally, you can pass in a new StartTime for the Stopwatch (leave as 0.0f or do not pass any parameters and the StartTime will remain the same).
	void Lap(float NewTime = 0.0f);

	// If a timer is set to recycle, call Stop() to end the recycle flag. By default, calling Stop() will stop the timer immediately. Pass false to choose to allow the current timer to expire before stopping.
	void Stop(bool ShouldDisable = true);

	// Test to see if the timer has gone off. Returns true if it has and then restarts the timer.
	bool Test();
};

