#include "StdAfx.h"
#include "Common/System/IInput.h"

#include "Common/Core/Defs.h"

#include <windows.h>

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{

	bool IInput::IsKeyPressed(EKeyCode keycode)
	{
		// TODO: Seagull's keycode to win32 keycode
		SHORT ret = ::GetAsyncKeyState(gKeyCodeToPlatformMap[keycode]);
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