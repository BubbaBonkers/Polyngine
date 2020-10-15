#pragma once

#include "PolynWin.h"
#include "PSystem/PInputManager/PInputManager.h"
#include "PRender/PEnvironment/PEnvironment.h"

class PCamera;

class Window
{
private:
	// Singleton static window manages registration and cleanup of this window class.
	class WindowClass
	{
	public:
		static const char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;

	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;
		static constexpr const char* WindowClassName = "Polyn Engine";
		static WindowClass WndClass;
		HINSTANCE hInst;
	};

public:
	Window(int Width, int Height, const char* Name, PInputManager* P_InputManager, PEnvironment* RendEnv) noexcept;
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	PInputManager* InputManager = nullptr;
	PEnvironment* RenderEnv = nullptr;

private:
	static LRESULT CALLBACK HandleMessageSetup(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMessageThunk(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) noexcept;

private:
	int Width;
	int Height;
	HWND hWind;
};
