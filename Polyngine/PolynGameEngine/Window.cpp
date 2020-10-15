#include "Window.h"
#include "Winuser.h"

#include "PRender/GUIToolbox/ImGui/imgui.h"
#include "PRender/GUIToolbox/ImGui/imgui_impl_win32.h"
#include "PRender/GUIToolbox/ImGui/imgui_impl_dx11.h"

// Window Class Stuff.
Window::WindowClass Window::WindowClass::WndClass;

// On construct, create a WNDCLASSEX for a new Window, then register it with Windows.
Window::WindowClass::WindowClass() noexcept : hInst( GetModuleHandle( nullptr ) )
{
	// Create the windows class for the main renderer window.
	WNDCLASSEX WindowClass = { 0 };
	WindowClass.cbSize = sizeof(WindowClass);
	WindowClass.style = CS_OWNDC | CS_DBLCLKS;
	WindowClass.lpfnWndProc = HandleMessageSetup;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetInstance();
	WindowClass.hIcon = nullptr;
	WindowClass.hCursor = nullptr;
	WindowClass.hbrBackground = CreateSolidBrush(0x00000000);
	WindowClass.lpszMenuName = nullptr;
	WindowClass.lpszClassName = GetName();
	WindowClass.hIconSm = nullptr;

	// Register the window class to Windows.
	RegisterClassEx(&WindowClass);
}

// On destruct, unregister this window.
Window::WindowClass::~WindowClass()
{
	UnregisterClass(WindowClassName, GetInstance());
}

// Return the name as a const char* of this Window.
const char* Window::WindowClass::GetName() noexcept
{
	return WindowClassName;
}

// Return the HINSTANCE of the current Window.
HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return WndClass.hInst;
}


// Window Stuff.

// On construct, the window will be setup using the input Width, Height, and Name.
Window::Window(int width, int height, const char* name, PInputManager* P_InputManager, PEnvironment* RendEnv) noexcept
{
	RECT WindowRectangle;
	WindowRectangle.left = 100;
	WindowRectangle.right = (width + WindowRectangle.left);
	WindowRectangle.top = 100;
	WindowRectangle.bottom = (height + WindowRectangle.top);
	AdjustWindowRect(&WindowRectangle, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	InputManager = P_InputManager;
	RenderEnv = RendEnv;

	// Create the main renderer window.
	InputManager->hwnd = CreateWindow(
		WindowClass::GetName(), name,
		WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_CLIPCHILDREN | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, (WindowRectangle.right - WindowRectangle.left), (WindowRectangle.bottom - WindowRectangle.top),
		nullptr, nullptr, WindowClass::GetInstance(), this
	);

	RenderEnv->hInst = WindowClass::GetInstance();

	// Show the window on the screen. SW_SHOWDEFAULT
	ShowWindow(InputManager->hwnd, SW_MAXIMIZE);
	UpdateWindow(InputManager->hwnd);
}

// When destructor is called, the window will destroy itself.
Window::~Window()
{
	// Destroy the window.
	DestroyWindow(hWind);
}

// Simply create the pointer to this instance for use in HandleMessageThunk().
LRESULT WINAPI Window::HandleMessageSetup(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	// Use the parameter passed in from CreateWindow() to save a pointer to the window class.
	if (Message == WM_NCCREATE)
	{
		// Cast window class from pointer.
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWindow = static_cast<Window*>(pCreate->lpCreateParams);

		// Set WinAPI managed data to store the pointer to the class window.
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));

		// Set the message procedure to non-setup hanlder now that the setup is complete.
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMessageThunk));

		// Send the message to the window class handler.
		return pWindow->HandleMessage(hWnd, Message, wParam, lParam);
	}
	
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

// Using the window pointer set in HandleMessageSetup(), send the message to the class handler.
LRESULT WINAPI Window::HandleMessageThunk(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	// Get the pointer to the window class that was set in HandleMessageSetup().
	Window* const pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	// Send the message to the class handler for data manipulation.
	return pWindow->HandleMessage(hWnd, Message, wParam, lParam);
}

// Link ImGui Proc.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Handle a message that is sent from HandleMessageThunk().
LRESULT Window::HandleMessage(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
	{
		return true;
	}

	switch (Message)
	{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_SIZE:
		{
			// Refresh the render window camera's aspect ratio.
			INT nWidth = LOWORD(lParam);
			INT nHeight = HIWORD(lParam);

			RenderEnv->RefreshCameraAspectRatios(((float)nWidth / (float)nHeight));
		}
	}

	if (InputManager != nullptr)
	{
		InputManager->QueueMessage(hWnd, Message, wParam, lParam);
	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}