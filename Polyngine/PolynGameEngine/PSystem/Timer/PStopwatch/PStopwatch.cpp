#include "PStopwatch.h"

PStopwatch::PStopwatch()
{
	bDisable = true;
}

PStopwatch::PStopwatch(float Time, bool bShouldRecycle)
{
	bDisable = false;
	Expiration = StartTime = Time;
	bRecycle = bShouldRecycle;
}

void PStopwatch::Start(float Time, bool bShouldRecycle)
{
	bDisable = false;
	Expiration = StartTime = Time;
	bRecycle = bShouldRecycle;
}

void PStopwatch::Update(float DeltaTime)
{
	if (!bDisable)
	{
		// If the Expiration has not reached 0, decrease it by the DeltaTime.
		if (Expiration > 0.0f)
		{
			Expiration -= DeltaTime;
		}
	}
}

float PStopwatch::GetTimeAlive()
{
	return (StartTime - Expiration);
}

void PStopwatch::Stop(bool ShouldDisable)
{
	if (!bDisable)
	{
		bRecycle = false;

		if (bDisable)
		{
			Expiration = 0.0f;
			bDisable = true;
		}
	}
}

bool PStopwatch::Test()
{
	if (!bDisable)
	{
		// If the Expiration has not reached 0, decrease it by the DeltaTime.
		if (Expiration <= 0.0f)
		{
			Expiration = StartTime;
			Cycles++;

			if (!bRecycle)
			{
				bDisable = true;
			}

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void PStopwatch::Lap(float NewTime)
{
	if (NewTime > 0.0f)
	{
		StartTime = NewTime;
	}

	Expiration = StartTime;
	Cycles++;
}