#include "Windows.h"

#include <vector>
#include <bitset>
#include <queue>
#include "Windowsx.h"
#include "../../PMath/PMath.h"

#pragma once

#define P_INPUT_MAX_SLOTS 300

// The PInputManager is responsible for catching input from the Window and storing all input in a queue between frames. At the beginning of each frame, the input queue will be processed and all input will be updated. You can use a PController (and attach it to a PInputManager) and possess any PObject derived class with the controller. The controller will communicate directly with the InputManager.
class PInputManager
{
public:
	// This enum will convert an input button to its counterpart in the
	// PInputStates array.
	enum class PInputMap
	{
		// Mouse Input
		PIN_MOUSE_L = 0,
		PIN_MOUSE_R = 1,
		PIN_MOUSE_M = 2,
		PIN_MOUSE_WHEEL_UP = 102,
		PIN_MOUSE_WHEEL_DOWN = 103,

		// Special Keyboard Keys
		PIN_SPACEBAR = 3,
		PIN_ENTER = 4,
		PIN_SHIFT = 99,
		PIN_SHIFT_L = 5,
		PIN_SHIFT_R = 6,
		PIN_CONTROL = 100,
		PIN_CONTROL_L = 7,
		PIN_CONTROL_R = 8,
		PIN_CAPSLOCK = 9,
		PIN_ESCAPE = 10,
		PIN_PAGEUP = 11,
		PIN_PAGEDOWN = 12,
		PIN_END = 13,
		PIN_HOME = 14,
		PIN_ARROW_L = 15,
		PIN_ARROW_UP = 16,
		PIN_ARROW_R = 17,
		PIN_ARROW_DOWN = 18,
		PIN_SELECT = 19,
		PIN_PRINTSCREEN = 20,
		PIN_INSERT = 21,
		PIN_DELETE = 22,
		PIN_HELP = 23,
		PIN_MULTIPLY = 44,
		PIN_ADD = 45,
		PIN_SEPARATOR = 46,
		PIN_SUBTRACT = 47,
		PIN_DECIMAL = 48,
		PIN_DIVIDE = 49,
		PIN_NUMLOCK = 74,
		PIN_SCROLLLOCK = 75,
		PIN_MENU_L = 76,
		PIN_MENU_R = 77,
		PIN_SEMICOLON = 86,
		PIN_PLUS = 87,
		PIN_COMMA = 88,
		PIN_MINUS = 89,
		PIN_PERIOD = 90,
		PIN_SLASH_FORWARD = 91,
		PIN_SLASH_BACKWARD = 92,
		PIN_TILDE = 93,
		PIN_BRACE_LEFT = 94,
		PIN_BRACE_RIGHT = 95,
		PIN_QUOTE = 96,
		PIN_PLAY = 97,
		PIN_TAB = 98,
		PIN_BACKSPACE = 101,

		// Numerical Keyboard Keys
		PIN_0 = 24,
		PIN_1 = 25,
		PIN_2 = 26,
		PIN_3 = 27,
		PIN_4 = 28,
		PIN_5 = 29,
		PIN_6 = 30,
		PIN_7 = 31,
		PIN_8 = 32,
		PIN_9 = 33,

		// Numerical Numpad Keys
		PIN_NUM_0 = 34,
		PIN_NUM_1 = 35,
		PIN_NUM_2 = 36,
		PIN_NUM_3 = 37,
		PIN_NUM_4 = 38,
		PIN_NUM_5 = 39,
		PIN_NUM_6 = 40,
		PIN_NUM_7 = 41,
		PIN_NUM_8 = 42,
		PIN_NUM_9 = 43,

		// Function Keyboard Keys
		PIN_F1 = 50,
		PIN_F2 = 51,
		PIN_F3 = 52,
		PIN_F4 = 53,
		PIN_F5 = 54,
		PIN_F6 = 55,
		PIN_F7 = 56,
		PIN_F8 = 57,
		PIN_F9 = 58,
		PIN_F10 = 59,
		PIN_F11 = 60,
		PIN_F12 = 61,
		PIN_F13 = 62,
		PIN_F14 = 63,
		PIN_F15 = 64,
		PIN_F16 = 65,
		PIN_F17 = 66,
		PIN_F18 = 67,
		PIN_F19 = 68,
		PIN_F20 = 69,
		PIN_F21 = 70,
		PIN_F22 = 71,
		PIN_F23 = 72,
		PIN_F24 = 73,

		// Alphabetical Keyboard Keys
		PIN_Q = 150,
		PIN_W = 151,
		PIN_E = 152,
		PIN_R = 153,
		PIN_T = 154,
		PIN_Y = 155,
		PIN_U = 156,
		PIN_I = 157,
		PIN_O = 158,
		PIN_P = 159,
		PIN_A = 160,
		PIN_S = 161,
		PIN_D = 162,
		PIN_F = 163,
		PIN_G = 164,
		PIN_H = 165,
		PIN_J = 166,
		PIN_K = 167,
		PIN_L = 168,
		PIN_Z = 169,
		PIN_X = 170,
		PIN_C = 171,
		PIN_V = 172,
		PIN_B = 173,
		PIN_N = 174,
		PIN_M = 175,

		// Music Keyboard Keys
		PIN_VOLUME_MUTE = 78,
		PIN_VOLUME_DOWN = 79,
		PIN_VOLUME_UP = 80,
		PIN_TRACK_NEXT = 81,
		PIN_TRACK_PREVIOUS = 82,
		PIN_MEDIA_STOP = 83,
		PIN_MEDIA_PAUSE_PLAY = 85,

		// Windows Specific Keys
		PIN_WINDOWS_L = 200,
		PIN_WINDOWS_R = 201,
		PIN_WINDOWS_APPS = 202
	};

private:
	// A structure that holds message information for the message queue.
	struct PMessage
	{
		UINT P_Msg;
		WPARAM wParam;
		LPARAM lParam;
	};

	// Input States is an array of PKeystate types. It holds information
	// for each input's states.
	//
	// [0] Key Down
	// [1] Key Up Last Frame
	// [2] Key Down Last Frame
	std::bitset<3> PInputStates[P_INPUT_MAX_SLOTS];

	float ScrollDelta = 0.0f;

	// Mouse-look movement.
	bool bResetRendererMousePos = false;
	float CurrMouseLoc[2]		= { 0, 0 };
	float PrevMouseLoc[2]		= { 0, 0 };
	PMath::float2 MouseDelta	= { 0, 0 };

public:
	// Queue that holds all input between frames.
	// All messages within are processed every frame update.
	std::queue<PMessage> InputQueue;
	HWND hwnd = NULL;

public:
	void QueueMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
	void ProcessInputQueue();
	void Update(float DeltaTime);

	bool IsInputDown( PInputMap Map );
	bool IsAnyInputDown( );
	bool WasInputHeldLastFrame(PInputMap Map);

	float GetScrollDelta( );
	PMath::float2 GetMouseDelta( );

private:
	void ProcessMessage( PMessage Msg );
};

