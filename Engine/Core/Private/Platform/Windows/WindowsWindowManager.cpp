#include "StdAfx.h"
#include "Platform/OS.h"

#include "System/Input.h"
#include "System/Logger.h"
#include "Memory/Memory.h"

namespace SG
{

	static int gPrevKey = -1;

	// default window process callback
	static LRESULT CALLBACK _WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		bool bPropagateOnDef = true;
		bool bWindowSizeChanging = false;

		switch (msg)
		{
		// on window destroy and app quit
		case WM_DESTROY:
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_KEYDOWN:
		{
			if (gPrevKey == wParam)
				Input::OnSystemKeyInputEvent(gPlatformToKeyCodeMap[wParam], EKeyState::eHold);
			else
				Input::OnSystemKeyInputEvent(gPlatformToKeyCodeMap[wParam], EKeyState::ePressed);
			gPrevKey = (int)wParam;
			break;
		}
		case WM_KEYUP:
		{
			if (gPrevKey == wParam) // reset
				gPrevKey = -1;
			Input::OnSystemKeyInputEvent(gPlatformToKeyCodeMap[wParam], EKeyState::eRelease);
			if (wParam == VK_ESCAPE)
				PostQuitMessage(0);
			break;
		}
		case WM_LBUTTONUP:
		{
			Input::OnSystemMouseKeyInputEvent(KeyCode_MouseLeft, EKeyState::eRelease);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			Input::OnSystemMouseKeyInputEvent(KeyCode_MouseLeft, EKeyState::ePressed);
			break;
		}
		case WM_MOUSEMOVE:
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			Input::OnSystemMouseMoveInputEvent(xPos, yPos);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			int direction = GET_Y_LPARAM(wParam);
			Input::OnSystemMouseWheelInputEvent(direction / 120);
			break;
		}
		// on window move and resize
		case WM_MOVE:
		{
			SSystem()->mMessageBus.PushEvent(ESystemMessage::eWindowMove);
			break;
		}
		case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED)
			{
				if (bWindowSizeChanging || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
				{
					SSystem()->mMessageBus.PushEvent(ESystemMessage::eWindowResize);
				}
			}
			else
			{
				SSystem()->mMessageBus.PushEvent(ESystemMessage::eWindowMinimal);
			}
			break;
		}
		case WM_ENTERSIZEMOVE:
		{
			bWindowSizeChanging = true;
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			bWindowSizeChanging = false;
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

	void WindowManager::OnInit(Monitor* const pMonitor)
	{
		// register a window class
		WNDCLASSEX wc;
		HINSTANCE instance = (HINSTANCE)::GetModuleHandle(NULL);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = SG::_WinProc;
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

		mMainWindow = Memory::New<Window>(pMonitor, L"Seagull Engine");
	}

	void WindowManager::OnShutdown()
	{
		Memory::Delete(mMainWindow);
	}

	SG::Window* WindowManager::GetMainWindow() const
	{
		return mMainWindow;
	}

}