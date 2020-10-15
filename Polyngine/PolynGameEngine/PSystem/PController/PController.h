#pragma once

#include "../PInputManager/PInputManager.h"

// The PController class is responsible to communicating input states with the attached PObject. When this is attached to a PObject, you are able to gain input control and move about the 3D world.
class PController
{
private:
	PInputManager* InputManager = nullptr;

public:
	PController(PInputManager* Input);
	HWND GetHWND();
	void Update(float DeltaTime);

	bool IsInputDown(PInputManager::PInputMap Map);	// Returns true if the in key is held down.
	bool WasInputHeldLastFrame(PInputManager::PInputMap Map);
	bool IsAnyInputDown();							// Returns true if any key is held down.
	PMath::float2 GetMouseDelta();					// Return mouse delta between previous frame.
	float GetScrollDelta();
	
	unsigned int RenderState = 0;
};

