#include "StdAfx.h"
#include "Common/Platform/IOperatingSystem.h"

#include "Common/Core/Defs.h"
#include "Common/System/ILog.h"

#include "Core/Stl/string.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

namespace SG
{
#ifdef SG_PLATFORM_WINDOWS
	///////////////////////////////////////////////////////////////////////////
	///  global functions of operating system
	///////////////////////////////////////////////////////////////////////////

	Vec2 GetCurrDpiScale()
	{
		HDC hdc = ::GetDC(NULL);
		const float dpiScale = 96.0f; // TODO: maybe this can be set somewhere
		Vec2 dpi;
		if (hdc)
		{
			dpi.x = static_cast<UINT>(::GetDeviceCaps(hdc, LOGPIXELSX)) / dpiScale;
			dpi.y = static_cast<UINT>(::GetDeviceCaps(hdc, LOGPIXELSY)) / dpiScale;
		}
		else
		{
			float systemDpi = ::GetDpiForSystem() / 96.0f;
			dpi.x = systemDpi;
			dpi.y = systemDpi;
		}
		::ReleaseDC(NULL, hdc);
		return dpi;
	}

	EOsMessage PeekOSMessage()
	{
		MSG msg = {};
		EOsMessage osMeg = EOsMessage::eNull;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);

			if (WM_CLOSE == msg.message || WM_QUIT == msg.message)
				osMeg = EOsMessage::eQuit;
		}
		return osMeg;
	}

	///////////////////////////////////////////////////////////////////////////
	///  global functions of operating system
	///////////////////////////////////////////////////////////////////////////
#endif
}