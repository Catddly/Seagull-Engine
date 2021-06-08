#include "StdAfx.h"
#include "MouseOpWindows.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

namespace SG
{

	SG::Vec2 CWindowsMouseOp::GetMousePosAbsolute() const
	{
		POINT pos;
		::GetCursorPos(&pos);
		return { (float)pos.x, (float)pos.y };
	}

	SG::Vec2 CWindowsMouseOp::GetMousePosRelative(SWindow* const pWindow) const
	{
		POINT pos;
		::GetCursorPos(&pos);
		ClientToScreen((HWND)pWindow->handle, &pos);
		return { (float)pos.x, (float)pos.y };
	}

}