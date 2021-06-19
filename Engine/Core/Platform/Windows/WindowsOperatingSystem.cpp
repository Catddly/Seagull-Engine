#include "StdAfx.h"
#include "Common/Platform/IOperatingSystem.h"

#include "Common/Core/Defs.h"
#include "Common/System/ILog.h"

#include "Common/Stl/string.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#ifdef SG_PLATFORM_WINDOWS
namespace SG
{
	///////////////////////////////////////////////////////////////////////////
	///  global functions of operating system
	///////////////////////////////////////////////////////////////////////////

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

	void PeekLastOSError()
	{
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID != ERROR_CLASS_ALREADY_EXISTS)
		{
			LPSTR messageBuffer = NULL;
			Size size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
			string message(messageBuffer, size);
			SG_LOG_ERROR("%s", message.c_str());
		}
	}

	///////////////////////////////////////////////////////////////////////////
	///  global functions of operating system
	///////////////////////////////////////////////////////////////////////////
}
#endif