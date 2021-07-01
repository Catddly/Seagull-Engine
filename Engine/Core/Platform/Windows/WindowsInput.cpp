#include "StdAfx.h"
#include "Common/System/IInput.h"

#include "Common/Core/Defs.h"

#include <windows.h>

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

	static int gKeyCodeToWin32Map[(UInt32)EKeyCode::KEYCODE_COUNT] = {
		VK_BACK,
		VK_TAB,
		VK_CLEAR,
		VK_RETURN,
		VK_LSHIFT,
		VK_RSHIFT,
		VK_SHIFT,
		VK_LCONTROL,
		VK_RCONTROL,
		VK_CONTROL,
		VK_LMENU,
		VK_RMENU,
		VK_MENU,
		VK_BROWSER_BACK,
		VK_BROWSER_FORWARD,
		VK_BROWSER_REFRESH,
		VK_BROWSER_STOP,
		VK_BROWSER_SEARCH,
		VK_BROWSER_FAVORITES,
		VK_BROWSER_HOME,
		VK_PAUSE,
		VK_CAPITAL,
		VK_ESCAPE,
		VK_SPACE,
		VK_PRIOR,
		VK_NEXT,
		VK_END,
		VK_HOME,
		VK_LEFT,
		VK_UP,
		VK_RIGHT,
		VK_DOWN,
		VK_SELECT,
		VK_PRINT,
		VK_EXECUTE,
		VK_SNAPSHOT,
		VK_INSERT,
		VK_DELETE,
		VK_HELP,
		VK_LWIN,
		VK_RWIN,
		VK_APPS,
		VK_SLEEP,
		VK_SCROLL,
		VK_VOLUME_MUTE,
		VK_VOLUME_UP,
		VK_VOLUME_DOWN,
		VK_NUMPAD0,
		VK_NUMPAD1,
		VK_NUMPAD2,
		VK_NUMPAD3,
		VK_NUMPAD4,
		VK_NUMPAD5,
		VK_NUMPAD6,
		VK_NUMPAD7,
		VK_NUMPAD8,
		VK_NUMPAD9,
		VK_MULTIPLY,
		VK_ADD,
		VK_SEPARATOR,
		VK_SUBTRACT,
		VK_DECIMAL,
		VK_DIVIDE,
		VK_NUMLOCK,
		0x30,
		0x31,
		0x32,
		0x33,
		0x34,
		0x35,
		0x36,
		0x37,
		0x38,
		0x39,
		0x41,
		0x42,
		0x43,
		0x44,
		0x45,
		0x46,
		0x47,
		0x48,
		0x49,
		0x4A,
		0x4B,
		0x4C,
		0x4D,
		0x4E,
		0x4F,
		0x50,
		0x51,
		0x52,
		0x53,
		0x54,
		0x55,
		0x56,
		0x57,
		0x58,
		0x59,
		0x5A,
		VK_F1,
		VK_F2,
		VK_F3,
		VK_F4,
		VK_F5,
		VK_F6,
		VK_F7,
		VK_F8,
		VK_F9,
		VK_F10,
		VK_F11,
		VK_F12,
		VK_F13,
		VK_F14,
		VK_F15,
		VK_F16,
		VK_F17,
		VK_F18,
		VK_F19,
		VK_F20,
		VK_F21,
		VK_F22,
		VK_F23,
		VK_F24,
	};

	bool IInput::IsKeyPressed(EKeyCode keycode)
	{
		// TODO: Seagull's keycode to win32 keycode
		SHORT ret = ::GetAsyncKeyState(gKeyCodeToWin32Map[(UInt32)keycode]);
		// Get the MSB
#if   SG_BIG_ENDIAN
		return ret & 0x80;
#elif SG_LITTLE_ENDIAN
		return ret & 0x01;
#else
#	error Unknown endian (Maybe the middle-endian)
#endif
	}

}
#endif // SG_PLATFORM_WINDOWS