#include "PInputManager.h"
#include <iostream>

// Adds a message to the InputQueue to be Processed on the next frame change.
void PInputManager::QueueMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	const PMessage P_Message = { Msg, wParam, lParam };
	InputQueue.push(P_Message);
}

// Process a single waiting message, always the front item, then pop it.
void PInputManager::ProcessMessage(PMessage Msg)
{
	// Switch on the message passed into the function.
	switch (Msg.P_Msg)
	{
		// Read keys that are held down.
		case WM_KEYDOWN:
		{
			switch (Msg.wParam)
			{
				// -------------------------------->
				// Mouse Input
				// -------------------------------->

				case 0x04:				// Middle Mouse Button
				{
					PInputStates[(int)PInputMap::PIN_MOUSE_M].set(0, true);
					break;
				}

				// -------------------------------->
				// Special Keyboard Keys
				// -------------------------------->

				case 0x20:				// Spacebar
				{
					PInputStates[(int)PInputMap::PIN_SPACEBAR].set(0, true);
					break;
				}

				case 0x0D:				// Enter
				{
					PInputStates[(int)PInputMap::PIN_ENTER].set(0, true);
					break;
				}

				case 0x10:				// Shift
				{
					PInputStates[(int)PInputMap::PIN_SHIFT].set(0, true);
					break;
				}

				case 0xA0:				// Left Shift
				{
					PInputStates[(int)PInputMap::PIN_SHIFT_L].set(0, true);
					break;
				}

				case 0xA1:				// Right Shift
				{
					PInputStates[(int)PInputMap::PIN_SHIFT_R].set(0, true);
					break;
				}

				case 0x11:				// Control
				{
					PInputStates[(int)PInputMap::PIN_CONTROL].set(0, true);
					break;
				}

				case 0xA2:				// Left Control
				{
					PInputStates[(int)PInputMap::PIN_CONTROL_L].set(0, true);
					break;
				}

				case 0xA3:				// Right Control
				{
					PInputStates[(int)PInputMap::PIN_CONTROL_R].set(0, true);
					break;
				}

				case 0x14:				// Caps Lock
				{
					PInputStates[(int)PInputMap::PIN_CAPSLOCK].set(0, true);
					break;
				}

				case 0x1B:				// Escape
				{
					PInputStates[(int)PInputMap::PIN_ESCAPE].set(0, true);
					break;
				}

				case 0x21:				// Page Up
				{
					PInputStates[(int)PInputMap::PIN_PAGEUP].set(0, true);
					break;
				}

				case 0x22:				// Page Down
				{
					PInputStates[(int)PInputMap::PIN_PAGEDOWN].set(0, true);
					break;
				}

				case 0x23:				// End
				{
					PInputStates[(int)PInputMap::PIN_END].set(0, true);
					break;
				}

				case 0x24:				// Home
				{
					PInputStates[(int)PInputMap::PIN_HOME].set(0, true);
					break;
				}

				case 0x25:				// Left Arrow
				{
					PInputStates[(int)PInputMap::PIN_ARROW_L].set(0, true);
					break;
				}

				case 0x26:				// Up Arrow
				{
					PInputStates[(int)PInputMap::PIN_ARROW_UP].set(0, true);
					break;
				}

				case 0x27:				// Right Arrow
				{
					PInputStates[(int)PInputMap::PIN_ARROW_R].set(0, true);
					break;
				}

				case 0x28:				// Down Arrow
				{
					PInputStates[(int)PInputMap::PIN_ARROW_DOWN].set(0, true);
					break;
				}

				case 0x29:				// Select
				{
					PInputStates[(int)PInputMap::PIN_SELECT].set(0, true);
					break;
				}

				case 0x2C:				// Print Screen
				{
					PInputStates[(int)PInputMap::PIN_PRINTSCREEN].set(0, true);
					break;
				}

				case 0x2D:				// Insert
				{
					PInputStates[(int)PInputMap::PIN_INSERT].set(0, true);
					break;
				}

				case 0x2E:				// Delete
				{
					PInputStates[(int)PInputMap::PIN_DELETE].set(0, true);
					break;
				}

				case 0x2F:				// Help
				{
					PInputStates[(int)PInputMap::PIN_HELP].set(0, true);
					break;
				}

				case 0x6A:				// Multiply
				{
					PInputStates[(int)PInputMap::PIN_MULTIPLY].set(0, true);
					break;
				}

				case 0x6B:				// Add
				{
					PInputStates[(int)PInputMap::PIN_ADD].set(0, true);
					break;
				}

				case 0x6C:				// Separator
				{
					PInputStates[(int)PInputMap::PIN_SEPARATOR].set(0, true);
					break;
				}

				case 0x6D:				// Subtract
				{
					PInputStates[(int)PInputMap::PIN_SUBTRACT].set(0, true);
					break;
				}

				case 0x6E:				// Decimal
				{
					PInputStates[(int)PInputMap::PIN_DECIMAL].set(0, true);
					break;
				}

				case 0x6F:				// Divide
				{
					PInputStates[(int)PInputMap::PIN_DIVIDE].set(0, true);
					break;
				}

				case 0x90:				// Num Lock
				{
					PInputStates[(int)PInputMap::PIN_NUMLOCK].set(0, true);
					break;
				}

				case 0x91:				// Scroll Lock
				{
					PInputStates[(int)PInputMap::PIN_SCROLLLOCK].set(0, true);
					break;
				}

				case 0xA4:				// Left Menu Key
				{
					PInputStates[(int)PInputMap::PIN_MENU_L].set(0, true);
					break;
				}

				case 0xA5:				// Right Menu Key
				{
					PInputStates[(int)PInputMap::PIN_MENU_R].set(0, true);
					break;
				}

				case 0xBA:				// ;: Semicolon
				{
					PInputStates[(int)PInputMap::PIN_SEMICOLON].set(0, true);
					break;
				}

				case 0xBB:				// + Plus
				{
					PInputStates[(int)PInputMap::PIN_PLUS].set(0, true);
					break;
				}

				case 0xBC:				// , Comma
				{
					PInputStates[(int)PInputMap::PIN_COMMA].set(0, true);
					break;
				}

				case 0xBD:				// - Minus
				{
					PInputStates[(int)PInputMap::PIN_MINUS].set(0, true);
					break;
				}

				case 0xBE:				// . Period
				{
					PInputStates[(int)PInputMap::PIN_PERIOD].set(0, true);
					break;
				}

				case 0xBF:				// /? Forward Slash/Question Mark
				{
					PInputStates[(int)PInputMap::PIN_SLASH_FORWARD].set(0, true);
					break;
				}

				case 0xDC:				// \| Backslash/Separator
				{
					PInputStates[(int)PInputMap::PIN_SLASH_BACKWARD].set(0, true);
					break;
				}

				case 0xC0:				// `~ Asterisk/Tilde
				{
					PInputStates[(int)PInputMap::PIN_TILDE].set(0, true);
					break;
				}

				case 0xDB:				// [{ Left Brace/Left Curly
				{
					PInputStates[(int)PInputMap::PIN_BRACE_LEFT].set(0, true);
					break;
				}

				case 0xDD:				// ]} Right Brace/Right Curly
				{
					PInputStates[(int)PInputMap::PIN_BRACE_RIGHT].set(0, true);
					break;
				}

				case 0xDE:				// '" Apostrophe/Quotation Mark
				{
					PInputStates[(int)PInputMap::PIN_QUOTE].set(0, true);
					break;
				}

				case 0xFA:				// Play
				{
					PInputStates[(int)PInputMap::PIN_PLAY].set(0, true);
					break;
				}

				case 0x09:				// Tab
				{
					PInputStates[(int)PInputMap::PIN_TAB].set(0, true);
					break;
				}

				case 0x08:				// Backspace
				{
					PInputStates[(int)PInputMap::PIN_BACKSPACE].set(0, true);
					break;
				}

				// -------------------------------->
				// Function Keyboard Keys
				// -------------------------------->

				case 0x70:				// F1
				{
					PInputStates[(int)PInputMap::PIN_F1].set(0, true);
					break;
				}

				case 0x71:				// F2
				{
					PInputStates[(int)PInputMap::PIN_F2].set(0, true);
					break;
				}

				case 0x72:				// F3
				{
					PInputStates[(int)PInputMap::PIN_F3].set(0, true);
					break;
				}

				case 0x73:				// F4
				{
					PInputStates[(int)PInputMap::PIN_F4].set(0, true);
					break;
				}

				case 0x74:				// F5
				{
					PInputStates[(int)PInputMap::PIN_F5].set(0, true);
					break;
				}

				case 0x75:				// F6
				{
					PInputStates[(int)PInputMap::PIN_F6].set(0, true);
					break;
				}

				case 0x76:				// F7
				{
					PInputStates[(int)PInputMap::PIN_F7].set(0, true);
					break;
				}

				case 0x77:				// F8
				{
					PInputStates[(int)PInputMap::PIN_F8].set(0, true);
					break;
				}

				case 0x78:				// F9
				{
					PInputStates[(int)PInputMap::PIN_F9].set(0, true);
					break;
				}

				case 0x79:				// F10
				{
					PInputStates[(int)PInputMap::PIN_F10].set(0, true);
					break;
				}

				case 0x7A:				// F11
				{
					PInputStates[(int)PInputMap::PIN_F11].set(0, true);
					break;
				}

				case 0x7B:				// F12
				{
					PInputStates[(int)PInputMap::PIN_F12].set(0, true);
					break;
				}

				case 0x7C:				// F13
				{
					PInputStates[(int)PInputMap::PIN_F13].set(0, true);
					break;
				}

				case 0x7D:				// F14
				{
					PInputStates[(int)PInputMap::PIN_F14].set(0, true);
					break;
				}

				case 0x7E:				// F15
				{
					PInputStates[(int)PInputMap::PIN_F15].set(0, true);
					break;
				}

				case 0x7F:				// F16
				{
					PInputStates[(int)PInputMap::PIN_F16].set(0, true);
					break;
				}

				case 0x80:				// F17
				{
					PInputStates[(int)PInputMap::PIN_F17].set(0, true);
					break;
				}

				case 0x81:				// F18
				{
					PInputStates[(int)PInputMap::PIN_F18].set(0, true);
					break;
				}

				case 0x82:				// F19
				{
					PInputStates[(int)PInputMap::PIN_F19].set(0, true);
					break;
				}

				case 0x83:				// F20
				{
					PInputStates[(int)PInputMap::PIN_F20].set(0, true);
					break;
				}

				case 0x84:				// F21
				{
					PInputStates[(int)PInputMap::PIN_F21].set(0, true);
					break;
				}

				case 0x85:				// F22
				{
					PInputStates[(int)PInputMap::PIN_F22].set(0, true);
					break;
				}

				case 0x86:				// F23
				{
					PInputStates[(int)PInputMap::PIN_F23].set(0, true);
					break;
				}

				case 0x87:				// F24
				{
					PInputStates[(int)PInputMap::PIN_F24].set(0, true);
					break;
				}

				// -------------------------------->
				// Numerical Keyboard Keys
				// -------------------------------->

				case 0x30:				// 0
				{
					PInputStates[(int)PInputMap::PIN_0].set(0, true);
					break;
				}

				case 0x31:				// 1
				{
					PInputStates[(int)PInputMap::PIN_1].set(0, true);
					break;
				}

				case 0x32:				// 2
				{
					PInputStates[(int)PInputMap::PIN_2].set(0, true);
					break;
				}

				case 0x33:				// 3
				{
					PInputStates[(int)PInputMap::PIN_3].set(0, true);
					break;
				}

				case 0x34:				// 4
				{
					PInputStates[(int)PInputMap::PIN_4].set(0, true);
					break;
				}

				case 0x35:				// 5
				{
					PInputStates[(int)PInputMap::PIN_5].set(0, true);
					break;
				}

				case 0x36:				// 6
				{
					PInputStates[(int)PInputMap::PIN_6].set(0, true);
					break;
				}

				case 0x37:				// 7
				{
					PInputStates[(int)PInputMap::PIN_7].set(0, true);
					break;
				}

				case 0x38:				// 8
				{
					PInputStates[(int)PInputMap::PIN_8].set(0, true);
					break;
				}

				case 0x39:				// 9
				{
					PInputStates[(int)PInputMap::PIN_9].set(0, true);
					break;
				}

				// -------------------------------->
				// Numerical Numpad Keys
				// -------------------------------->

				case 0x60:				// Numpad 0
				{
					PInputStates[(int)PInputMap::PIN_NUM_0].set(0, true);
					break;
				}

				case 0x61:				// Numpad 1
				{
					PInputStates[(int)PInputMap::PIN_NUM_1].set(0, true);
					break;
				}

				case 0x62:				// Numpad 2
				{
					PInputStates[(int)PInputMap::PIN_NUM_2].set(0, true);
					break;
				}

				case 0x63:				// Numpad 3
				{
					PInputStates[(int)PInputMap::PIN_NUM_3].set(0, true);
					break;
				}

				case 0x64:				// Numpad 4
				{
					PInputStates[(int)PInputMap::PIN_NUM_4].set(0, true);
					break;
				}

				case 0x65:				// Numpad 5
				{
					PInputStates[(int)PInputMap::PIN_NUM_5].set(0, true);
					break;
				}

				case 0x66:				// Numpad 6
				{
					PInputStates[(int)PInputMap::PIN_NUM_6].set(0, true);
					break;
				}

				case 0x67:				// Numpad 7
				{
					PInputStates[(int)PInputMap::PIN_NUM_7].set(0, true);
					break;
				}

				case 0x68:				// Numpad 8
				{
					PInputStates[(int)PInputMap::PIN_NUM_8].set(0, true);
					break;
				}

				case 0x69:				// Numpad 9
				{
					PInputStates[(int)PInputMap::PIN_NUM_9].set(0, true);
					break;
				}

				// -------------------------------->
				// Alphabetical Keyboard Keys
				// -------------------------------->

				case 0x51:				// Q
				{
					PInputStates[(int)PInputMap::PIN_Q].set(0, true);
					break;
				}

				case 0x57:				// W
				{
					PInputStates[(int)PInputMap::PIN_W].set(0, true);
					break;
				}

				case 0x45:				// E
				{
					PInputStates[(int)PInputMap::PIN_E].set(0, true);
					break;
				}

				case 0x52:				// R
				{
					PInputStates[(int)PInputMap::PIN_R].set(0, true);
					break;
				}

				case 0x54:				// T
				{
					PInputStates[(int)PInputMap::PIN_T].set(0, true);
					break;
				}

				case 0x59:				// Y
				{
					PInputStates[(int)PInputMap::PIN_Y].set(0, true);
					break;
				}

				case 0x55:				// U
				{
					PInputStates[(int)PInputMap::PIN_U].set(0, true);
					break;
				}

				case 0x49:				// I
				{
					PInputStates[(int)PInputMap::PIN_I].set(0, true);
					break;
				}

				case 0x4F:				// O
				{
					PInputStates[(int)PInputMap::PIN_O].set(0, true);
					break;
				}

				case 0x50:				// P
				{
					PInputStates[(int)PInputMap::PIN_P].set(0, true);
					break;
				}

				case 0x41:				// A
				{
					PInputStates[(int)PInputMap::PIN_A].set(0, true);
					break;
				}

				case 0x53:				// S
				{
					PInputStates[(int)PInputMap::PIN_S].set(0, true);
					break;
				}

				case 0x44:				// D
				{
					PInputStates[(int)PInputMap::PIN_D].set(0, true);
					break;
				}

				case 0x46:				// F
				{
					PInputStates[(int)PInputMap::PIN_F].set(0, true);
					break;
				}

				case 0x47:				// G
				{
					PInputStates[(int)PInputMap::PIN_G].set(0, true);
					break;
				}

				case 0x48:				// H
				{
					PInputStates[(int)PInputMap::PIN_H].set(0, true);
					break;
				}

				case 0x4A:				// J
				{
					PInputStates[(int)PInputMap::PIN_J].set(0, true);
					break;
				}

				case 0x4B:				// K
				{
					PInputStates[(int)PInputMap::PIN_K].set(0, true);
					break;
				}

				case 0x4C:				// L
				{
					PInputStates[(int)PInputMap::PIN_L].set(0, true);
					break;
				}

				case 0x5A:				// Z
				{
					PInputStates[(int)PInputMap::PIN_Z].set(0, true);
					break;
				}

				case 0x58:				// X
				{
					PInputStates[(int)PInputMap::PIN_X].set(0, true);
					break;
				}

				case 0x43:				// C
				{
					PInputStates[(int)PInputMap::PIN_C].set(0, true);
					break;
				}

				case 0x56:				// V
				{
					PInputStates[(int)PInputMap::PIN_V].set(0, true);
					break;
				}

				case 0x42:				// B
				{
					PInputStates[(int)PInputMap::PIN_B].set(0, true);
					break;
				}

				case 0x4E:				// N
				{
					PInputStates[(int)PInputMap::PIN_N].set(0, true);
					break;
				}

				case 0x4D:				// M
				{
					PInputStates[(int)PInputMap::PIN_M].set(0, true);
					break;
				}

				// -------------------------------->
				// Music Keyboard Keys
				// -------------------------------->

				case 0xAD:				// Mute Volume
				{
					PInputStates[(int)PInputMap::PIN_VOLUME_MUTE].set(0, true);
					break;
				}

				case 0xAE:				// Volume Down
				{
					PInputStates[(int)PInputMap::PIN_VOLUME_DOWN].set(0, true);
					break;
				}

				case 0xAF:				// Volume Up
				{
					PInputStates[(int)PInputMap::PIN_VOLUME_UP].set(0, true);
					break;
				}

				case 0xB0:				// Next Track
				{
					PInputStates[(int)PInputMap::PIN_TRACK_NEXT].set(0, true);
					break;
				}

				case 0xB1:				// Previous Track
				{
					PInputStates[(int)PInputMap::PIN_TRACK_PREVIOUS].set(0, true);
					break;
				}

				case 0xB2:				// Stop Media
				{
					PInputStates[(int)PInputMap::PIN_MEDIA_STOP].set(0, true);
					break;
				}

				case 0xB3:				// Play/Pause Media
				{
					PInputStates[(int)PInputMap::PIN_MEDIA_PAUSE_PLAY].set(0, true);
					break;
				}

				// -------------------------------->
				// Windows Specific Keys
				// -------------------------------->

				case 0x5B:				// Left Windows
				{
					PInputStates[(int)PInputMap::PIN_WINDOWS_L].set(0, true);
					break;
				}

				case 0x5C:				// Right Windows
				{
					PInputStates[(int)PInputMap::PIN_WINDOWS_L].set(0, true);
					break;
				}

				case 0x5D:				// Windows Apps
				{
					PInputStates[(int)PInputMap::PIN_WINDOWS_APPS].set(0, true);
					break;
				}
			}
			break;
		}

		// Read the keys that are not held down.
		case WM_KEYUP:
		{
			switch (Msg.wParam)
			{
				// -------------------------------->
				// Mouse Input
				// -------------------------------->

				case 0x04:				// Middle Mouse Button
				{
					PInputStates[(int)PInputMap::PIN_MOUSE_M].set(0, false);
					break;
				}

				// -------------------------------->
				// Special Keyboard Keys
				// -------------------------------->

				case 0x20:				// Spacebar
				{
					PInputStates[(int)PInputMap::PIN_SPACEBAR].set(0, false);
					break;
				}

				case 0x0D:				// Enter
				{
					PInputStates[(int)PInputMap::PIN_ENTER].set(0, false);
					break;
				}

				case 0x10:				// Shift
				{
					PInputStates[(int)PInputMap::PIN_SHIFT].set(0, false);
					break;
				}

				case 0xA0:				// Left Shift
				{
					PInputStates[(int)PInputMap::PIN_SHIFT_L].set(0, false);
					break;
				}

				case 0xA1:				// Right Shift
				{
					PInputStates[(int)PInputMap::PIN_SHIFT_R].set(0, false);
					break;
				}

				case 0x11:				// Control
				{
					PInputStates[(int)PInputMap::PIN_CONTROL].set(0, false);
					break;
				}

				case 0xA2:				// Left Control
				{
					PInputStates[(int)PInputMap::PIN_CONTROL_L].set(0, false);
					break;
				}

				case 0xA3:				// Right Control
				{
					PInputStates[(int)PInputMap::PIN_CONTROL_R].set(0, false);
					break;
				}

				case 0x14:				// Caps Lock
				{
					PInputStates[(int)PInputMap::PIN_CAPSLOCK].set(0, false);
					break;
				}

				case 0x1B:				// Escape
				{
					PInputStates[(int)PInputMap::PIN_ESCAPE].set(0, false);
					break;
				}

				case 0x21:				// Page Up
				{
					PInputStates[(int)PInputMap::PIN_PAGEUP].set(0, false);
					break;
				}

				case 0x22:				// Page Down
				{
					PInputStates[(int)PInputMap::PIN_PAGEDOWN].set(0, false);
					break;
				}

				case 0x23:				// End
				{
					PInputStates[(int)PInputMap::PIN_END].set(0, false);
					break;
				}

				case 0x24:				// Home
				{
					PInputStates[(int)PInputMap::PIN_HOME].set(0, false);
					break;
				}

				case 0x25:				// Left Arrow
				{
					PInputStates[(int)PInputMap::PIN_ARROW_L].set(0, false);
					break;
				}

				case 0x26:				// Up Arrow
				{
					PInputStates[(int)PInputMap::PIN_ARROW_UP].set(0, false);
					break;
				}

				case 0x27:				// Right Arrow
				{
					PInputStates[(int)PInputMap::PIN_ARROW_R].set(0, false);
					break;
				}

				case 0x28:				// Down Arrow
				{
					PInputStates[(int)PInputMap::PIN_ARROW_DOWN].set(0, false);
					break;
				}

				case 0x29:				// Select
				{
					PInputStates[(int)PInputMap::PIN_SELECT].set(0, false);
					break;
				}

				case 0x2C:				// Print Screen
				{
					PInputStates[(int)PInputMap::PIN_PRINTSCREEN].set(0, false);
					break;
				}

				case 0x2D:				// Insert
				{
					PInputStates[(int)PInputMap::PIN_INSERT].set(0, false);
					break;
				}

				case 0x2E:				// Delete
				{
					PInputStates[(int)PInputMap::PIN_DELETE].set(0, false);
					break;
				}

				case 0x2F:				// Help
				{
					PInputStates[(int)PInputMap::PIN_HELP].set(0, false);
					break;
				}

				case 0x6A:				// Multiply
				{
					PInputStates[(int)PInputMap::PIN_MULTIPLY].set(0, false);
					break;
				}

				case 0x6B:				// Add
				{
					PInputStates[(int)PInputMap::PIN_ADD].set(0, false);
					break;
				}

				case 0x6C:				// Separator
				{
					PInputStates[(int)PInputMap::PIN_SEPARATOR].set(0, false);
					break;
				}

				case 0x6D:				// Subtract
				{
					PInputStates[(int)PInputMap::PIN_SUBTRACT].set(0, false);
					break;
				}

				case 0x6E:				// Decimal
				{
					PInputStates[(int)PInputMap::PIN_DECIMAL].set(0, false);
					break;
				}

				case 0x6F:				// Divide
				{
					PInputStates[(int)PInputMap::PIN_DIVIDE].set(0, false);
					break;
				}

				case 0x90:				// Num Lock
				{
					PInputStates[(int)PInputMap::PIN_NUMLOCK].set(0, false);
					break;
				}

				case 0x91:				// Scroll Lock
				{
					PInputStates[(int)PInputMap::PIN_SCROLLLOCK].set(0, false);
					break;
				}

				case 0xA4:				// Left Menu Key
				{
					PInputStates[(int)PInputMap::PIN_MENU_L].set(0, false);
					break;
				}

				case 0xA5:				// Right Menu Key
				{
					PInputStates[(int)PInputMap::PIN_MENU_R].set(0, false);
					break;
				}

				case 0xBA:				// ;: Semicolon
				{
					PInputStates[(int)PInputMap::PIN_SEMICOLON].set(0, false);
					break;
				}

				case 0xBB:				// + Plus
				{
					PInputStates[(int)PInputMap::PIN_PLUS].set(0, false);
					break;
				}

				case 0xBC:				// , Comma
				{
					PInputStates[(int)PInputMap::PIN_COMMA].set(0, false);
					break;
				}

				case 0xBD:				// - Minus
				{
					PInputStates[(int)PInputMap::PIN_MINUS].set(0, false);
					break;
				}

				case 0xBE:				// . Period
				{
					PInputStates[(int)PInputMap::PIN_PERIOD].set(0, false);
					break;
				}

				case 0xBF:				// /? Forward Slash/Question Mark
				{
					PInputStates[(int)PInputMap::PIN_SLASH_FORWARD].set(0, false);
					break;
				}

				case 0xDC:				// \| Backslash/Separator
				{
					PInputStates[(int)PInputMap::PIN_SLASH_BACKWARD].set(0, false);
					break;
				}

				case 0xC0:				// `~ Asterisk/Tilde
				{
					PInputStates[(int)PInputMap::PIN_TILDE].set(0, false);
					break;
				}

				case 0xDB:				// [{ Left Brace/Left Curly
				{
					PInputStates[(int)PInputMap::PIN_BRACE_LEFT].set(0, false);
					break;
				}

				case 0xDD:				// ]} Right Brace/Right Curly
				{
					PInputStates[(int)PInputMap::PIN_BRACE_RIGHT].set(0, false);
					break;
				}

				case 0xDE:				// '" Apostrophe/Quotation Mark
				{
					PInputStates[(int)PInputMap::PIN_QUOTE].set(0, false);
					break;
				}

				case 0xFA:				// Play
				{
					PInputStates[(int)PInputMap::PIN_PLAY].set(0, false);
					break;
				}

				case 0x09:				// Tab
				{
					PInputStates[(int)PInputMap::PIN_TAB].set(0, false);
					break;
				}

				case 0x08:				// Backspace
				{
					PInputStates[(int)PInputMap::PIN_BACKSPACE].set(0, false);
					break;
				}

				// -------------------------------->
				// Function Keyboard Keys
				// -------------------------------->

				case 0x70:				// F1
				{
					PInputStates[(int)PInputMap::PIN_F1].set(0, false);
					break;
				}

				case 0x71:				// F2
				{
					PInputStates[(int)PInputMap::PIN_F2].set(0, false);
					break;
				}

				case 0x72:				// F3
				{
					PInputStates[(int)PInputMap::PIN_F3].set(0, false);
					break;
				}

				case 0x73:				// F4
				{
					PInputStates[(int)PInputMap::PIN_F4].set(0, false);
					break;
				}

				case 0x74:				// F5
				{
					PInputStates[(int)PInputMap::PIN_F5].set(0, false);
					break;
				}

				case 0x75:				// F6
				{
					PInputStates[(int)PInputMap::PIN_F6].set(0, false);
					break;
				}

				case 0x76:				// F7
				{
					PInputStates[(int)PInputMap::PIN_F7].set(0, false);
					break;
				}

				case 0x77:				// F8
				{
					PInputStates[(int)PInputMap::PIN_F8].set(0, false);
					break;
				}

				case 0x78:				// F9
				{
					PInputStates[(int)PInputMap::PIN_F9].set(0, false);
					break;
				}

				case 0x79:				// F10
				{
					PInputStates[(int)PInputMap::PIN_F10].set(0, false);
					break;
				}

				case 0x7A:				// F11
				{
					PInputStates[(int)PInputMap::PIN_F11].set(0, false);
					break;
				}

				case 0x7B:				// F12
				{
					PInputStates[(int)PInputMap::PIN_F12].set(0, false);
					break;
				}

				case 0x7C:				// F13
				{
					PInputStates[(int)PInputMap::PIN_F13].set(0, false);
					break;
				}

				case 0x7D:				// F14
				{
					PInputStates[(int)PInputMap::PIN_F14].set(0, false);
					break;
				}

				case 0x7E:				// F15
				{
					PInputStates[(int)PInputMap::PIN_F15].set(0, false);
					break;
				}

				case 0x7F:				// F16
				{
					PInputStates[(int)PInputMap::PIN_F16].set(0, false);
					break;
				}

				case 0x80:				// F17
				{
					PInputStates[(int)PInputMap::PIN_F17].set(0, false);
					break;
				}

				case 0x81:				// F18
				{
					PInputStates[(int)PInputMap::PIN_F18].set(0, false);
					break;
				}

				case 0x82:				// F19
				{
					PInputStates[(int)PInputMap::PIN_F19].set(0, false);
					break;
				}

				case 0x83:				// F20
				{
					PInputStates[(int)PInputMap::PIN_F20].set(0, false);
					break;
				}

				case 0x84:				// F21
				{
					PInputStates[(int)PInputMap::PIN_F21].set(0, false);
					break;
				}

				case 0x85:				// F22
				{
					PInputStates[(int)PInputMap::PIN_F22].set(0, false);
					break;
				}

				case 0x86:				// F23
				{
					PInputStates[(int)PInputMap::PIN_F23].set(0, false);
					break;
				}

				case 0x87:				// F24
				{
					PInputStates[(int)PInputMap::PIN_F24].set(0, false);
					break;
				}

				// -------------------------------->
				// Numerical Keyboard Keys
				// -------------------------------->

				case 0x30:				// 0
				{
					PInputStates[(int)PInputMap::PIN_0].set(0, false);
					break;
				}

				case 0x31:				// 1
				{
					PInputStates[(int)PInputMap::PIN_1].set(0, false);
					break;
				}

				case 0x32:				// 2
				{
					PInputStates[(int)PInputMap::PIN_2].set(0, false);
					break;
				}

				case 0x33:				// 3
				{
					PInputStates[(int)PInputMap::PIN_3].set(0, false);
					break;
				}

				case 0x34:				// 4
				{
					PInputStates[(int)PInputMap::PIN_4].set(0, false);
					break;
				}

				case 0x35:				// 5
				{
					PInputStates[(int)PInputMap::PIN_5].set(0, false);
					break;
				}

				case 0x36:				// 6
				{
					PInputStates[(int)PInputMap::PIN_6].set(0, false);
					break;
				}

				case 0x37:				// 7
				{
					PInputStates[(int)PInputMap::PIN_7].set(0, false);
					break;
				}

				case 0x38:				// 8
				{
					PInputStates[(int)PInputMap::PIN_8].set(0, false);
					break;
				}

				case 0x39:				// 9
				{
					PInputStates[(int)PInputMap::PIN_9].set(0, false);
					break;
				}

				// -------------------------------->
				// Numerical Numpad Keys
				// -------------------------------->

				case 0x60:				// Numpad 0
				{
					PInputStates[(int)PInputMap::PIN_NUM_0].set(0, false);
					break;
				}

				case 0x61:				// Numpad 1
				{
					PInputStates[(int)PInputMap::PIN_NUM_1].set(0, false);
					break;
				}

				case 0x62:				// Numpad 2
				{
					PInputStates[(int)PInputMap::PIN_NUM_2].set(0, false);
					break;
				}

				case 0x63:				// Numpad 3
				{
					PInputStates[(int)PInputMap::PIN_NUM_3].set(0, false);
					break;
				}

				case 0x64:				// Numpad 4
				{
					PInputStates[(int)PInputMap::PIN_NUM_4].set(0, false);
					break;
				}

				case 0x65:				// Numpad 5
				{
					PInputStates[(int)PInputMap::PIN_NUM_5].set(0, false);
					break;
				}

				case 0x66:				// Numpad 6
				{
					PInputStates[(int)PInputMap::PIN_NUM_6].set(0, false);
					break;
				}

				case 0x67:				// Numpad 7
				{
					PInputStates[(int)PInputMap::PIN_NUM_7].set(0, false);
					break;
				}

				case 0x68:				// Numpad 8
				{
					PInputStates[(int)PInputMap::PIN_NUM_8].set(0, false);
					break;
				}

				case 0x69:				// Numpad 9
				{
					PInputStates[(int)PInputMap::PIN_NUM_9].set(0, false);
					break;
				}

				// -------------------------------->
				// Alphabetical Keyboard Keys
				// -------------------------------->

				case 0x51:				// Q
				{
					PInputStates[(int)PInputMap::PIN_Q].set(0, false);
					break;
				}

				case 0x57:				// W
				{
					PInputStates[(int)PInputMap::PIN_W].set(0, false);
					break;
				}

				case 0x45:				// E
				{
					PInputStates[(int)PInputMap::PIN_E].set(0, false);
					break;
				}

				case 0x52:				// R
				{
					PInputStates[(int)PInputMap::PIN_R].set(0, false);
					break;
				}

				case 0x54:				// T
				{
					PInputStates[(int)PInputMap::PIN_T].set(0, false);
					break;
				}

				case 0x59:				// Y
				{
					PInputStates[(int)PInputMap::PIN_Y].set(0, false);
					break;
				}

				case 0x55:				// U
				{
					PInputStates[(int)PInputMap::PIN_U].set(0, false);
					break;
				}

				case 0x49:				// I
				{
					PInputStates[(int)PInputMap::PIN_I].set(0, false);
					break;
				}

				case 0x4F:				// O
				{
					PInputStates[(int)PInputMap::PIN_O].set(0, false);
					break;
				}

				case 0x50:				// P
				{
					PInputStates[(int)PInputMap::PIN_P].set(0, false);
					break;
				}

				case 0x41:				// A
				{
					PInputStates[(int)PInputMap::PIN_A].set(0, false);
					break;
				}

				case 0x53:				// S
				{
					PInputStates[(int)PInputMap::PIN_S].set(0, false);
					break;
				}

				case 0x44:				// D
				{
					PInputStates[(int)PInputMap::PIN_D].set(0, false);
					break;
				}

				case 0x46:				// F
				{
					PInputStates[(int)PInputMap::PIN_F].set(0, false);
					break;
				}

				case 0x47:				// G
				{
					PInputStates[(int)PInputMap::PIN_G].set(0, false);
					break;
				}

				case 0x48:				// H
				{
					PInputStates[(int)PInputMap::PIN_H].set(0, false);
					break;
				}

				case 0x4A:				// J
				{
					PInputStates[(int)PInputMap::PIN_J].set(0, false);
					break;
				}

				case 0x4B:				// K
				{
					PInputStates[(int)PInputMap::PIN_K].set(0, false);
					break;
				}

				case 0x4C:				// L
				{
					PInputStates[(int)PInputMap::PIN_L].set(0, false);
					break;
				}

				case 0x5A:				// Z
				{
					PInputStates[(int)PInputMap::PIN_Z].set(0, false);
					break;
				}

				case 0x58:				// X
				{
					PInputStates[(int)PInputMap::PIN_X].set(0, false);
					break;
				}

				case 0x43:				// C
				{
					PInputStates[(int)PInputMap::PIN_C].set(0, false);
					break;
				}

				case 0x56:				// V
				{
					PInputStates[(int)PInputMap::PIN_V].set(0, false);
					break;
				}

				case 0x42:				// B
				{
					PInputStates[(int)PInputMap::PIN_B].set(0, false);
					break;
				}

				case 0x4E:				// N
				{
					PInputStates[(int)PInputMap::PIN_N].set(0, false);
					break;
				}

				case 0x4D:				// M
				{
					PInputStates[(int)PInputMap::PIN_M].set(0, false);
					break;
				}

				// -------------------------------->
				// Music Keyboard Keys
				// -------------------------------->

				case 0xAD:				// Mute Volume
				{
					PInputStates[(int)PInputMap::PIN_VOLUME_MUTE].set(0, false);
					break;
				}

				case 0xAE:				// Volume Down
				{
					PInputStates[(int)PInputMap::PIN_VOLUME_DOWN].set(0, false);
					break;
				}

				case 0xAF:				// Volume Up
				{
					PInputStates[(int)PInputMap::PIN_VOLUME_UP].set(0, false);
					break;
				}

				case 0xB0:				// Next Track
				{
					PInputStates[(int)PInputMap::PIN_TRACK_NEXT].set(0, false);
					break;
				}

				case 0xB1:				// Previous Track
				{
					PInputStates[(int)PInputMap::PIN_TRACK_PREVIOUS].set(0, false);
					break;
				}

				case 0xB2:				// Stop Media
				{
					PInputStates[(int)PInputMap::PIN_MEDIA_STOP].set(0, false);
					break;
				}

				case 0xB3:				// Play/Pause Media
				{
					PInputStates[(int)PInputMap::PIN_MEDIA_PAUSE_PLAY].set(0, false);
					break;
				}

				// -------------------------------->
				// Windows Specific Keys
				// -------------------------------->

				case 0x5B:				// Left Windows
				{
					PInputStates[(int)PInputMap::PIN_WINDOWS_L].set(0, false);
					break;
				}

				case 0x5C:				// Right Windows
				{
					PInputStates[(int)PInputMap::PIN_WINDOWS_L].set(0, false);
					break;
				}

				case 0x5D:				// Windows Apps
				{
					PInputStates[(int)PInputMap::PIN_WINDOWS_APPS].set(0, false);
					break;
				}
			}
			break;
		}

		// Get left mouse button input.
		case WM_LBUTTONDOWN:
		{
			PInputStates[(int)PInputMap::PIN_MOUSE_L].set(0, true);
			break;
		}

		case WM_LBUTTONUP:
		{
			PInputStates[(int)PInputMap::PIN_MOUSE_L].set(0, false);
			break;
		}

		// Get right mouse button input.
		case WM_RBUTTONDOWN:
		{
			bResetRendererMousePos = true;

			PInputStates[(int)PInputMap::PIN_MOUSE_R].set(0, true);
			break;
		}

		case WM_RBUTTONUP:
		{
			bResetRendererMousePos = true;

			PInputStates[(int)PInputMap::PIN_MOUSE_R].set(0, false);
			break;
		}

		// Get mouse movement.
		case WM_MOUSEMOVE:
		{
			break;
		}

		// Get mouse wheel input.
		case WM_MOUSEWHEEL:
		{
			if ((short)HIWORD(Msg.wParam) > 0)
			{
				// Increase scroll value.
				ScrollDelta = 1.0f;
			}
			else
			{
				// Decrease scroll value.
				ScrollDelta = -1.0f;
			}

			break;
		}

		default:
		{
			break;
		}
	}

	InputQueue.pop();
}

// Process all waiting input messages.
void PInputManager::ProcessInputQueue()
{
	if (InputQueue.size() > 0)
	{
		for (unsigned int i = 0; i < InputQueue.size(); ++i)
		{
			// Process the message next in the queue.
			ProcessMessage(InputQueue.front());
		}
	}

	POINT MousePos;
	GetCursorPos(&MousePos);

	if (bResetRendererMousePos)
	{
		PrevMouseLoc[0] = CurrMouseLoc[0] = (float)MousePos.x;
		PrevMouseLoc[1] = CurrMouseLoc[1] = (float)MousePos.y;

		bResetRendererMousePos = false;
	}

	POINT TempMousePos = { (long)CurrMouseLoc[0], (long)CurrMouseLoc[1] };
	CurrMouseLoc[0] = (float)MousePos.x;
	CurrMouseLoc[1] = (float)MousePos.y;

	if ((CurrMouseLoc[0] != PrevMouseLoc[0]) || (CurrMouseLoc[1] != PrevMouseLoc[1]))
	{
		MouseDelta = { CurrMouseLoc[0] - TempMousePos.x, CurrMouseLoc[1] - TempMousePos.y };
	}

	PrevMouseLoc[0] = (float)TempMousePos.x;
	PrevMouseLoc[1] = (float)TempMousePos.y;
}

// Read the value of a key's entry. Returns true if the key is held down.
bool PInputManager::IsInputDown(PInputMap Map)
{
	return PInputStates[(int)Map].test(0);
}

// Returns true if any key is currently held down.
bool PInputManager::IsAnyInputDown()
{
	for (unsigned int i = 0; i < sizeof(PInputManager::PInputStates) / sizeof(PInputManager::PInputStates[0]); ++i)
	{
		if (PInputManager::PInputStates[i].any())
		{
			return true;
		}
	}

	return false;
}

// Returns true if the inputted key was held down last frame.
bool PInputManager::WasInputHeldLastFrame(PInputMap Map)
{
	return PInputStates[(int)Map].test(2);
}

// Return the delta from the mouse scroll wheel.
float PInputManager::GetScrollDelta()
{
	return PInputManager::ScrollDelta;
}

PMath::float2 PInputManager::GetMouseDelta()
{
	return MouseDelta;
}

// Update is called before ProcessInputQueue().
void PInputManager::Update(float DeltaTime)
{
	if (ScrollDelta > 0.0f)
	{
		ScrollDelta -= (DeltaTime * 2.0f);

		if (ScrollDelta > 0.25f)
		{
			PInputStates[(int)PInputMap::PIN_MOUSE_WHEEL_UP].set(0, true);
		}
		else
		{
			PInputStates[(int)PInputMap::PIN_MOUSE_WHEEL_UP].set(0, false);
		}
	}
	else if (ScrollDelta < 0.0f)
	{
		ScrollDelta += (DeltaTime * 2.0f);

		if (ScrollDelta < -0.25f)
		{
			PInputStates[(int)PInputMap::PIN_MOUSE_WHEEL_DOWN].set(0, true);
		}
		else
		{
			PInputStates[(int)PInputMap::PIN_MOUSE_WHEEL_DOWN].set(0, false);
		}
	}
	else
	{
		PInputStates[(int)PInputMap::PIN_MOUSE_WHEEL_UP].set(0, false);
		PInputStates[(int)PInputMap::PIN_MOUSE_WHEEL_DOWN].set(0, false);
	}
}