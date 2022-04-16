#include "StdAfx.h"
#include "Platform/OS.h"

#include "System/Logger.h"
#include "System/Input.h"
#include "System/System.h"
#include "Memory/Memory.h"

#if 0
#include <windows.h>
#endif

namespace SG
{

	static SG_INLINE WPARAM _DistinguishExtendKeycode(WPARAM keycode, LPARAM testBit)
	{
		switch (keycode)
		{
		case VK_SHIFT:
			keycode = MapVirtualKey(((testBit & 0x00ff0000) >> 16), MAPVK_VSC_TO_VK_EX);
			break;
		case VK_CONTROL:
			keycode = ((testBit & 0x01000000) != 0) ? VK_RCONTROL : VK_LCONTROL;
			break;
		case VK_MENU:
			keycode = ((testBit & 0x01000000) != 0) ? VK_RMENU : VK_LMENU;
			break;
		case VK_RETURN:
			keycode = ((testBit & 0x01000000) != 0) ? 0x8F : VK_RETURN;
			break;
		}
		return keycode;
	}

	// default window process callback
	static LRESULT CALLBACK _WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
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
		case WM_SYSKEYDOWN:
		{
			//SG_LOG_DEBUG("%d", wParam);
			Input::OnSystemKeyInputEvent(gPlatformToKeyCodeMap[_DistinguishExtendKeycode(wParam, lParam)], true);
			return 0; // block left alt and right alt message propagated to the ::DefWindowProc()
		}
		case WM_SYSKEYUP:
		{
			Input::OnSystemKeyInputEvent(gPlatformToKeyCodeMap[_DistinguishExtendKeycode(wParam, lParam)], false);
			return 0; // block left alt and right alt message propagated to the ::DefWindowProc()
		}
		case WM_KEYDOWN:
		{
			//SG_LOG_DEBUG("%d", wParam);
			Input::OnSystemKeyInputEvent(gPlatformToKeyCodeMap[_DistinguishExtendKeycode(wParam, lParam)], true);
			break;
		}
		case WM_KEYUP:
		{
			Input::OnSystemKeyInputEvent(gPlatformToKeyCodeMap[_DistinguishExtendKeycode(wParam, lParam)], false);
			if (wParam == VK_ESCAPE)
				PostQuitMessage(0);
			break;
		}
		case WM_LBUTTONUP:
		{
			Input::OnSystemKeyInputEvent(KeyCode_MouseLeft, false);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			Input::OnSystemKeyInputEvent(KeyCode_MouseLeft, true);
			break;
		}
		case WM_RBUTTONUP:
		{
			Input::OnSystemKeyInputEvent(KeyCode_MouseRight, false);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			Input::OnSystemKeyInputEvent(KeyCode_MouseRight, true);
			break;
		}
		case WM_MBUTTONUP:
		{
			Input::OnSystemKeyInputEvent(KeyCode_MouseMiddle, false);
			break;
		}
		case WM_MBUTTONDOWN:
		{
			Input::OnSystemKeyInputEvent(KeyCode_MouseMiddle, true);
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
			int direction = GET_WHEEL_DELTA_WPARAM(wParam);
			Input::OnSystemMouseWheelInputEvent(direction / 120);
			break;
		}
		// on window move and resize
		case WM_MOVE:
		{
			SSystem()->mSystemMessageManager.PushEvent(ESystemMessage::eWindowMove);
			break;
		}
		case WM_CHAR: // input acsii
		{
			DWORD character = static_cast<DWORD>(wParam);
			if (character <= 127) // ASCII
				Input::OnSystemCharInput(static_cast<char>(character));
			break;
		}
		case WM_IME_CHAR:
		{
			DWORD character = static_cast<DWORD>(wParam);
			if (character <= 127)
			{
				Input::OnSystemCharInput(static_cast<Char>(character));
			}
			else
			{
				// TODO: IME_CHAR should flip hi-byte and lo-byte
			}
			break;
		}
		case WM_UNICHAR:
		{
			if (wParam > 0 && wParam < 0x10000)
				Input::OnSystemWideCharInput(static_cast<WChar>(wParam));
			break;
		}
		case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED)
			{
				if (bWindowSizeChanging || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
				{
					SSystem()->mSystemMessageManager.PushEvent(ESystemMessage::eWindowResize);
				}
			}
			else
			{
				SSystem()->mSystemMessageManager.PushEvent(ESystemMessage::eWindowMinimal);
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
		return ::DefWindowProc(hwnd, msg, wParam, lParam);
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
		mSecondaryWindows.resize(0);
	}

	void WindowManager::OnShutdown()
	{
		for (auto* pWindow : mSecondaryWindows)
			Memory::Delete(pWindow);
		Memory::Delete(mMainWindow);
	}

	SG::Window* WindowManager::GetMainWindow() const
	{
		return mMainWindow;
	}

	SG::Window* WindowManager::CreateNewWindow(Monitor* const pMonitor)
	{
		auto* newWindow = Memory::New<Window>(pMonitor);
		mSecondaryWindows.push_back(newWindow);
		return newWindow;
	}

	void WindowManager::ShowMouseCursor() const
	{
		::ShowCursor(true);
	}

	void WindowManager::HideMouseCursor() const
	{
		::ShowCursor(false);
	}

	void WindowManager::SetMouseCursor(ECursorType type)
	{
		LPCWSTR cursor = IDC_ARROW;
		switch (type)
		{
		case SG::ECursorType::eArrow: cursor = IDC_ARROW; break;
		case SG::ECursorType::eTextInput: cursor = IDC_IBEAM; break;
		case SG::ECursorType::eResizeNS: cursor = IDC_SIZENS; break;
		case SG::ECursorType::eResizeWE: cursor = IDC_SIZEWE; break;
		case SG::ECursorType::eResizeNWSE: cursor = IDC_SIZENWSE; break;
		case SG::ECursorType::eResizeNESW: cursor = IDC_SIZENESW; break;
		case SG::ECursorType::eResizeAll: cursor = IDC_SIZEALL; break;
		case SG::ECursorType::eHand: cursor = IDC_HAND; break;
		case SG::ECursorType::eNoAllowed: cursor = IDC_NO; break;
		case SG::ECursorType::eHelp: cursor = IDC_HELP; break;
		case SG::ECursorType::eStarting: cursor = IDC_APPSTARTING; break;
		case SG::ECursorType::eWait: cursor = IDC_WAIT; break;
		case SG::ECursorType::MAX_NUM_CURSOR:
		default:
			SG_LOG_ERROR("Invalid cursor type!");
			break;
		}

		::SetCursor(LoadCursor(NULL, cursor));
	}

}