#include "StdAfx.h"
#include "Common/Platform/WindowManager.h"

#include "Common/Platform/DeviceManager.h"
#include "Common/Platform/Window.h"

#include "Common/System/ILog.h"
#include "Common/Memory/IMemory.h"

#include <EASTL/utility.h>

namespace SG
{

	// default window process callback
	static LRESULT CALLBACK _WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		bool bPropagateOnDef = true;
		switch (msg)
		{
		case WM_DESTROY:
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_KEYDOWN:
		{
			//SG_LOG_DEBUG("Keydown (%d)", wParam);
			break;
		}
		case WM_KEYUP:
		{
			//SG_LOG_DEBUG("Keyup (%d)", wParam);
			if (wParam == VK_ESCAPE)
				PostQuitMessage(0);
			break;
		}
		//case WM_NCLBUTTONDOWN:   // left mouse down on non-client area
		//	if (wParam != HTCLOSE) // you can not the drag window, you can only close the window
		//	{
		//		bPropagateOnDef = false;
		//		break;
		//	}
		}

		if (bPropagateOnDef)
			return ::DefWindowProc(hwnd, msg, wParam, lParam);
		else
			return 0;
	}

	void CWindowManager::OnInit(Monitor* const pMonitor)
	{
		// register a window class
		WNDCLASSEX wc;
		HINSTANCE instance = (HINSTANCE)::GetModuleHandle(NULL);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = _WinProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = instance;
		wc.hIcon   = ::LoadIcon(0, IDI_APPLICATION);
		wc.hIconSm = ::LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = ::LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = SG_ENGINE_WNAME;

		if (!::RegisterClassEx(&wc))
		{
			// get the error message, if any.
			PeekLastOSError();
		}

		mMainWindow = new Window(pMonitor, L"Seagull Engine");
	}

	void CWindowManager::OnShutdown()
	{
		delete mMainWindow;
	}

	SG::Window* CWindowManager::GetMainWindow() const
	{
		return mMainWindow;
	}

}