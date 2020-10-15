#include "PolynWin.h"
#include "Window.h"
#include "PSystem/Timer/Timer.h"
#include "PSystem/Timer/PStopwatch/PStopwatch.h"
#include "PRender/PRender.h"
#include <iostream>
#include "Window.h"
#include "Winuser.h"

// Forward declared functions.
void CreateConsole();
void DestroyConsole();
MSG StartMainLoop();

// One-time creations and camera for render window setup.
PInputManager InputManager;
Renderer::PRender RenderEngine;

// Catch windows processes and messages to manipulate data flow.
LRESULT CALLBACK WndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	// Switch on the message passed into the function.
	switch ( Msg )
	{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
	}
}

// Entry point for the program.
int CALLBACK WinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPSTR		lpCmdLine,
	_In_		int			nShowCmd )
{
	// Create the window and ready it for use. Ensure it matches the size of the desktop rectangle before maximizing.
	RECT Desktop;
	GetClientRect(GetDesktopWindow(), &Desktop);

	Window RenderWindow((int)Desktop.right, (int)Desktop.bottom, "Polyn v0.5", &InputManager, &RenderEngine.Environment);

	CreateConsole();

	MSG Message_L = StartMainLoop();

	DestroyConsole();

	// Return success and exit the program.
	return (int)Message_L.wParam;
}

void CreateConsole()
{
	AllocConsole();
	FILE* new_std_in_out;
	freopen_s(&new_std_in_out, "CONOUT$", "w", stdout);
	freopen_s(&new_std_in_out, "CONIN$", "r", stdin);
	std::cout << "Debug Console Opened.\n\n";
}

void DestroyConsole()
{
	FreeConsole();
}

MSG StartMainLoop()
{
	// Message struct and bool for return result from message.
	MSG Message;

	// Create the timer for time concurrency, then start it.
	// Create the global total game time variable to track total time in engine.
	Timer Clock;
	Clock.Start();
	float G_GameTime = 0.0f;
	PStopwatch SW_FullscreenToggle(1.0f, true);

	srand((unsigned int)time(NULL));

	// Setup required PInputManager and PCamera that are required to Initialize the renderer, then initialize it.
	RenderEngine.Environment.InputManager = &InputManager;
	RenderEngine.InitializeRenderer(InputManager.hwnd);

	int StartFullscreen = GetPrivateProfileInt("Renderer.Startup", "bStartInFullscreen", 0, "../Configurations/Engine.ini");

	// Check for start in fullscreen for program.
	if (StartFullscreen == 1)
	{
		std::cout << "Fullscreen: Enabled\n";
		RenderEngine.SwapChain->SetFullscreenState(true, nullptr);
	}

	// Main game loop.
	while (true)
	{
		// Notify the timer to restart to keep time for next frame.
		float DeltaTime = (float)(Clock.GetElapsedMiliseconds() * 0.001f);
		Clock.Restart();

		// Clear debug line renderer.
		ClearLines();

		// Notify the Input Manager to process all waiting input.
		InputManager.ProcessInputQueue();

		// Check for fullscreen toggle.
		if ((RenderEngine.Environment.CurrentState != ERenderStates::SHIP) && (InputManager.IsInputDown(PInputManager::PInputMap::PIN_F11) && SW_FullscreenToggle.Test()))
		{
			BOOL IsInFullScreen;
			RenderEngine.SwapChain->GetFullscreenState(&IsInFullScreen, nullptr);

			if (RenderEngine.GetFullscreenMode())
			{
				std::cout << "Fullscreen: Disabled\n";
				RenderEngine.SetFullscreenMode(false);
			}
			else
			{
				std::cout << "Fullscreen: Enabled\n";
				RenderEngine.SetFullscreenMode(true);
			}
		}

		// Increase the global game time.
		G_GameTime += ((float)Clock.GetElapsedMiliseconds() * 0.001f);

		// Update objects that need to be updated.
		InputManager.Update(DeltaTime);
		RenderEngine.Environment.Update(DeltaTime);
		SW_FullscreenToggle.Update(DeltaTime);

		// Process all messages, stop on WM_QUIT
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			// WM_QUIT does not need to be // translated or dispatched
			if (Message.message == WM_QUIT)
			{
				break;
			}

			// One of TranslateMessage()'s jobs is to translate input into WM_CHAR messages,
			// without TranslateMessage(), WM_CHAR messages seize to send through Windows.
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else
		{
			// Call to the renderer and draw the current frame.
			RenderEngine.Draw();
		}
	}

	// Clean up Renderer references.
	RenderEngine.DestroyRenderer();

	return Message;
}