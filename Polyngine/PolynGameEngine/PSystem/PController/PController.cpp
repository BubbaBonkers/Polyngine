#include "PController.h"
#include "../../PRender/GUIToolbox/ImGui/imgui.h"
#include "../../PRender/GUIToolbox/ImGui/imgui_impl_win32.h"
#include "../../PRender/GUIToolbox/ImGui/imgui_impl_dx11.h"

PController::PController(PInputManager* Input)
{
	InputManager = Input;
}

HWND PController::GetHWND()
{
	if (InputManager)
	{
		return InputManager->hwnd;
	}
	else
	{
		return NULL;
	}
}

void PController::Update(float DeltaTime)
{
	
}

bool PController::IsInputDown(PInputManager::PInputMap Map)
{
	if (RenderState != 0 || (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard))
	{
		if (InputManager)
		{
			return InputManager->IsInputDown(Map);
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

bool PController::WasInputHeldLastFrame(PInputManager::PInputMap Map)
{
	if (RenderState != 0 || (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard))
	{
		if (InputManager)
		{
			return InputManager->WasInputHeldLastFrame(Map);
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

bool PController::IsAnyInputDown()
{
	if (RenderState != 0 || (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard))
	{
		if (InputManager)
		{
			return InputManager->IsAnyInputDown();
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

PMath::float2 PController::GetMouseDelta()
{
	if (RenderState != 0 || (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard))
	{
		if (InputManager)
		{
			return InputManager->GetMouseDelta();
		}
		else
		{
			return { 0.0f, 0.0f };
		}
	}
	else
	{
		return { 0.0f, 0.0f };
	}
}

float PController::GetScrollDelta()
{
	if (RenderState != 0 || (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard))
	{
		if (InputManager)
		{
			return InputManager->GetScrollDelta();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}
