#include "StdAfx.h"
#include "ImGuiDriver.h"

#include "Scene/Scene.h"

#include "Platform/OS.h"
#include "System/Logger.h"
#include "System/Input.h"
#include "Memory/Memory.h"

#include "imgui/imgui.h"

namespace SG
{

	static ECursorType gCursorTypeMap[ImGuiMouseCursor_COUNT] = {
		ECursorType::eArrow,
		ECursorType::eTextInput,
		ECursorType::eResizeAll,
		ECursorType::eResizeNS,
		ECursorType::eResizeWE,
		ECursorType::eResizeNESW,
		ECursorType::eResizeNWSE,
		ECursorType::eHand,
		ECursorType::eNoAllowed
	};

	static ImGuiKey ToImguiKey(EKeyCode keycode)
	{
		switch (keycode)
		{
		case KeyCode_Tab: return ImGuiKey_Tab;
		case KeyCode_Left: return ImGuiKey_LeftArrow;
		case KeyCode_Right: return ImGuiKey_RightArrow;
		case KeyCode_Up: return ImGuiKey_UpArrow;
		case KeyCode_Down: return ImGuiKey_DownArrow;
		case KeyCode_PageUp: return ImGuiKey_PageUp;
		case KeyCode_PageDown: return ImGuiKey_PageDown;
		case KeyCode_Home: return ImGuiKey_Home;
		case KeyCode_End: return ImGuiKey_End;
		case KeyCode_Insert: return ImGuiKey_Insert;
		case KeyCode_Delete: return ImGuiKey_Delete;
		case KeyCode_Back: return ImGuiKey_Backspace;
		case KeyCode_Space: return ImGuiKey_Space;
		case KeyCode_Enter: return ImGuiKey_Enter;
		case KeyCode_Escape: return ImGuiKey_Escape;
		case KeyCode_Apostrophe: return ImGuiKey_Apostrophe;
		case KeyCode_Comma: return ImGuiKey_Comma;
		case KeyCode_Minus: return ImGuiKey_Minus;
		case KeyCode_Period: return ImGuiKey_Period;
		case KeyCode_Slash: return ImGuiKey_Slash;
		case KeyCode_Semicolon: return ImGuiKey_Semicolon;
		case KeyCode_Plus: return ImGuiKey_Equal;
		case KeyCode_LeftBracket: return ImGuiKey_LeftBracket;
		case KeyCode_BackSlash: return ImGuiKey_Backslash;
		case KeyCode_RightBracket: return ImGuiKey_RightBracket;
		case KeyCode_GraveAccent: return ImGuiKey_GraveAccent;
		case KeyCode_Capital: return ImGuiKey_CapsLock;
		case KeyCode_ScrollLock: return ImGuiKey_ScrollLock;
		case KeyCode_NumLock: return ImGuiKey_NumLock;
		case KeyCode_Print: return ImGuiKey_PrintScreen;
		case KeyCode_Pause: return ImGuiKey_Pause;
		case KeyCode_Numpad0: return ImGuiKey_Keypad0;
		case KeyCode_Numpad1: return ImGuiKey_Keypad1;
		case KeyCode_Numpad2: return ImGuiKey_Keypad2;
		case KeyCode_Numpad3: return ImGuiKey_Keypad3;
		case KeyCode_Numpad4: return ImGuiKey_Keypad4;
		case KeyCode_Numpad5: return ImGuiKey_Keypad5;
		case KeyCode_Numpad6: return ImGuiKey_Keypad6;
		case KeyCode_Numpad7: return ImGuiKey_Keypad7;
		case KeyCode_Numpad8: return ImGuiKey_Keypad8;
		case KeyCode_Numpad9: return ImGuiKey_Keypad9;
		case KeyCode_Decimal: return ImGuiKey_KeypadDecimal;
		case KeyCode_Divide: return ImGuiKey_KeypadDivide;
		case KeyCode_Multiply: return ImGuiKey_KeypadMultiply;
		case KeyCode_Subtract: return ImGuiKey_KeypadSubtract;
		case KeyCode_Add: return ImGuiKey_KeypadAdd;
		case KeyCode_NumpadEnter: return ImGuiKey_KeypadEnter;
		//case KeyCode_: return ImGuiKey_KeypadEqual;
		case KeyCode_LeftShift: return ImGuiKey_LeftShift;
		case KeyCode_LeftControl: return ImGuiKey_LeftCtrl;
		case KeyCode_LeftAlt: return ImGuiKey_LeftAlt;
		//case KeyCode_LeftSup: return ImGuiKey_LeftSuper;
		case KeyCode_RightShift: return ImGuiKey_RightShift;
		case KeyCode_RightControl: return ImGuiKey_RightCtrl;
		case KeyCode_RightAlt: return ImGuiKey_RightAlt;
		//case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
		case KeyCode_Alt: return ImGuiKey_Menu;
		case KeyCode_0: return ImGuiKey_0;
		case KeyCode_1: return ImGuiKey_1;
		case KeyCode_2: return ImGuiKey_2;
		case KeyCode_3: return ImGuiKey_3;
		case KeyCode_4: return ImGuiKey_4;
		case KeyCode_5: return ImGuiKey_5;
		case KeyCode_6: return ImGuiKey_6;
		case KeyCode_7: return ImGuiKey_7;
		case KeyCode_8: return ImGuiKey_8;
		case KeyCode_9: return ImGuiKey_9;
		case KeyCode_A: return ImGuiKey_A;
		case KeyCode_B: return ImGuiKey_B;
		case KeyCode_C: return ImGuiKey_C;
		case KeyCode_D: return ImGuiKey_D;
		case KeyCode_E: return ImGuiKey_E;
		case KeyCode_F: return ImGuiKey_F;
		case KeyCode_G: return ImGuiKey_G;
		case KeyCode_H: return ImGuiKey_H;
		case KeyCode_I: return ImGuiKey_I;
		case KeyCode_J: return ImGuiKey_J;
		case KeyCode_K: return ImGuiKey_K;
		case KeyCode_L: return ImGuiKey_L;
		case KeyCode_M: return ImGuiKey_M;
		case KeyCode_N: return ImGuiKey_N;
		case KeyCode_O: return ImGuiKey_O;
		case KeyCode_P: return ImGuiKey_P;
		case KeyCode_Q: return ImGuiKey_Q;
		case KeyCode_R: return ImGuiKey_R;
		case KeyCode_S: return ImGuiKey_S;
		case KeyCode_T: return ImGuiKey_T;
		case KeyCode_U: return ImGuiKey_U;
		case KeyCode_V: return ImGuiKey_V;
		case KeyCode_W: return ImGuiKey_W;
		case KeyCode_X: return ImGuiKey_X;
		case KeyCode_Y: return ImGuiKey_Y;
		case KeyCode_Z: return ImGuiKey_Z;
		case KeyCode_F1: return ImGuiKey_F1;
		case KeyCode_F2: return ImGuiKey_F2;
		case KeyCode_F3: return ImGuiKey_F3;
		case KeyCode_F4: return ImGuiKey_F4;
		case KeyCode_F5: return ImGuiKey_F5;
		case KeyCode_F6: return ImGuiKey_F6;
		case KeyCode_F7: return ImGuiKey_F7;
		case KeyCode_F8: return ImGuiKey_F8;
		case KeyCode_F9: return ImGuiKey_F9;
		case KeyCode_F10: return ImGuiKey_F10;
		case KeyCode_F11: return ImGuiKey_F11;
		case KeyCode_F12: return ImGuiKey_F12;
		default: return ImGuiKey_None;
		}
	}
	
	static ImGuiMouseButton ToImGuiMouseButton(EKeyCode keycode)
	{
		if (keycode == KeyCode_MouseLeft)
			return ImGuiMouseButton_Left;
		else if (keycode == KeyCode_MouseRight)
			return ImGuiMouseButton_Right;
		else if (keycode == KeyCode_MouseMiddle)
			return ImGuiMouseButton_Middle;
		return ImGuiMouseButton_Left;
	}

	static void* _ImGuiMemAllocFunc(Size sz, void* userData)
	{
		SG_NO_USE(userData);
		return Memory::Malloc(sz);
	}

	static void  _ImGuiMemFreeFunc(void* ptr, void* userData)
	{
		SG_NO_USE(userData);
		return Memory::Free(ptr);
	}

	static void _ImGui_Platform_CreateWindow_Impl(ImGuiViewport* vp)
	{
		auto* newWindow = OperatingSystem::CreateNewWindow();

		vp->PlatformHandle = newWindow;
		vp->PlatformHandleRaw = newWindow->GetNativeHandle();
	}

	static void _ImGui_Platform_DestroyWindow_Impl(ImGuiViewport* vp)
	{
		// Engnie handle the destruction of the window
		vp->PlatformHandle = nullptr;
		vp->PlatformHandleRaw = nullptr;
	}

	static void _ImGui_Platform_ShowWindow_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->ShowWindow();
	}

	static void _ImGui_Platform_SetWindowPos_Impl(ImGuiViewport* vp, ImVec2 pos)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->SetPosition((UInt32)pos.x, (UInt32)pos.y);
	}

	static ImVec2 _ImGui_Platform_GetWindowPos_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		auto& rect = pWindow->GetCurrRect();
		return { (float)rect.left, (float)rect.top };
	}

	static void _ImGui_Platform_SetWindowSize_Impl(ImGuiViewport* vp, ImVec2 size)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->SetSize((UInt32)size.x, (UInt32)size.y);
	}

	static ImVec2 _ImGui_Platform_GetWindowSize_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		return { (float)pWindow->GetWidth(), (float)pWindow->GetHeight() };
	}

	static void _ImGui_Platform_SetWindowFocus_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->SetFocus();
	}

	static bool _ImGui_Platform_GetWindowFocus_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		return pWindow->IsFocus();
	}

	static bool _ImGui_Platform_GetWindowMinimized_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		return pWindow->IsMinimize();
	}

	static void _ImGui_Platform_SetWindowTitle_Impl(ImGuiViewport* vp, const char* str)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->SetTitle(str);
	}

	static void _ImGui_Platform_SetClipboardText_Impl(void* pUserData, const char* text)
	{
		auto* pWindow = reinterpret_cast<Window*>(pUserData);
		pWindow->SetClipboardText(text);
	}

	static const char* _ImGui_Platform_GetClipboardText_Impl(void* pUserData)
	{
		auto* pWindow = reinterpret_cast<Window*>(pUserData);
		return pWindow->GetClipboardText();
	}

	static void _ImGuiBindWindowPlatformFunc()
	{
		// Register platform interface (will be coupled with a renderer interface)
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Platform_CreateWindow = _ImGui_Platform_CreateWindow_Impl;
		platformIO.Platform_DestroyWindow = _ImGui_Platform_DestroyWindow_Impl;
		platformIO.Platform_ShowWindow = _ImGui_Platform_ShowWindow_Impl;
		platformIO.Platform_SetWindowPos = _ImGui_Platform_SetWindowPos_Impl;
		platformIO.Platform_GetWindowPos = _ImGui_Platform_GetWindowPos_Impl;
		platformIO.Platform_SetWindowSize = _ImGui_Platform_SetWindowSize_Impl;
		platformIO.Platform_GetWindowSize = _ImGui_Platform_GetWindowSize_Impl;
		platformIO.Platform_SetWindowFocus = _ImGui_Platform_SetWindowFocus_Impl;
		platformIO.Platform_GetWindowFocus = _ImGui_Platform_GetWindowFocus_Impl;
		platformIO.Platform_GetWindowMinimized = _ImGui_Platform_GetWindowMinimized_Impl;
		platformIO.Platform_SetWindowTitle = _ImGui_Platform_SetWindowTitle_Impl;
		//platformIO.Platform_RenderWindow = NULL;
		//platformIO.Platform_SwapBuffers = NULL;
	}

	ImGuiDriver::ImGuiDriver()
	{
		Input::RegisterListener(EListenerPriority::eLevel0, this);
	}

	ImGuiDriver::~ImGuiDriver()
	{
		Input::RemoveListener(this);
	}

	bool ImGuiDriver::OnInit()
	{
		ImGui::SetAllocatorFunctions(_ImGuiMemAllocFunc, _ImGuiMemFreeFunc);
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // enable Multi-Viewport / Platform Windows

		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)
		//io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;  // We can create multi-viewports on the Platform side (optional)

		// for copy-paste functionality
		io.SetClipboardTextFn = _ImGui_Platform_SetClipboardText_Impl;
		io.GetClipboardTextFn = _ImGui_Platform_GetClipboardText_Impl;
		io.ClipboardUserData = OperatingSystem::GetMainWindow();

		ImGui::StyleColorsDark();

		// when viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;

			_ImGuiBindWindowPlatformFunc();
		}

		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Monitors.resize(0);

		ImGuiPlatformMonitor monitor;
		Rect& monitorRect = OperatingSystem::GetMainMonitor()->GetMonitorRect();
		monitor.MainPos = monitor.WorkPos = { (float)monitorRect.left, (float)monitorRect.top };
		monitor.MainSize = monitor.WorkSize = { (float)GetRectWidth(monitorRect), (float)GetRectHeight(monitorRect) };
		platformIO.Monitors.push_back(monitor);

		// Our mouse update function expect PlatformHandle to be filled for the main viewport
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		mainViewport->PlatformHandle = (void*)OperatingSystem::GetMainWindow();
		mainViewport->PlatformHandleRaw = OperatingSystem::GetMainWindow()->GetNativeHandle();

		return true;
	}

	void ImGuiDriver::OnShutdown()
	{
		ImGui::DestroyContext();
	}

	void ImGuiDriver::OnUpdate(float deltaTime)
	{
		UpdateFrameData(deltaTime);

		ImGui::NewFrame();
		mLayerSystem.OnUpdate(deltaTime); // do user draw
		ImGui::EndFrame();

		auto& io = ImGui::GetIO();
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiDriver::UpdateFrameData(float deltaTime)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = deltaTime;

		io.DisplaySize = { (float)OperatingSystem::GetMainWindow()->GetWidth(), (float)OperatingSystem::GetMainWindow()->GetHeight() };
		io.DisplayFramebufferScale = { 1.0f, 1.0f };

		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		for (int n = 0; n < platformIO.Viewports.Size; n++)
		{
			const ImGuiViewport* viewport = platformIO.Viewports[n];
			auto* pWindow = reinterpret_cast<Window*>(viewport->PlatformHandle);
			if (pWindow->IsMouseCursorInWindow())
			{
				io.AddMousePosEvent((float)mLastValidMousePos[0], (float)mLastValidMousePos[1]);
			}
			else
			{
				io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
			}
		}
		UpdateCursorData();
	}

	void ImGuiDriver::UpdateMouseData()
	{
		ImGuiIO& io = ImGui::GetIO();
		const ImVec2 mousePosPrev = io.MousePos;

		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		for (int n = 0; n < platformIO.Viewports.Size; n++)
		{
			const ImGuiViewport* viewport = platformIO.Viewports[n];
			auto* pWindow = reinterpret_cast<Window*>(viewport->PlatformHandle);

			const bool bIsWindowFocused = pWindow->IsFocus();

			if (bIsWindowFocused)
			{
				auto mousePos = OperatingSystem::GetMainWindow()->GetMousePosRelative();
				if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					// Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
					// Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
					auto& windowRect = pWindow->GetCurrRect();
					mousePos[0] += windowRect.left;
					mousePos[1] += windowRect.top;
				}

				io.AddMousePosEvent((float)mousePos[0], (float)mousePos[1]);
				//SG_LOG_DEBUG("%d, %d", mousePos[0], mousePos[1]);
			}
		}

		if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)
			io.AddMouseViewportEvent(0);
	}

	void ImGuiDriver::UpdateCursorData()
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
			return;

		ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		for (int n = 0; n < platformIO.Viewports.Size; n++)
		{
			if (imguiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
			{
				// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
				OperatingSystem::HideMouseCursor();
			}
			else
			{
				// Show OS mouse cursor
				OperatingSystem::SetMouseCursor(gCursorTypeMap[imguiCursor]);
				OperatingSystem::ShowMouseCursor();
			}
		}
	}

	// callback functions
	bool ImGuiDriver::OnKeyInputUpdate(EKeyCode keycode, EKeyState keyState)
	{
		//if (keyState == EKeyState::eHold)
		//	SG_LOG_DEBUG("Hold: %d", keycode);
		//else if (keyState == EKeyState::ePressed)
		//	SG_LOG_DEBUG("Pressed: %d", keycode);
		//else if (keyState == EKeyState::eRelease)
		//	SG_LOG_DEBUG("Release: %d", keycode);

		//if (keycode == KeyCode_LeftShift && keyState == EKeyState::ePressed)
		//	SG_LOG_DEBUG("Left Shift!");
		//if (keycode == KeyCode_RightShift && keyState == EKeyState::ePressed)
		//	SG_LOG_DEBUG("Right Shift!");
		//if (keycode == KeyCode_LeftControl && keyState == EKeyState::ePressed)
		//	SG_LOG_DEBUG("Left Control!");
		//if (keycode == KeyCode_RightControl && keyState == EKeyState::ePressed)
		//	SG_LOG_DEBUG("Right Control!");
		//if (keycode == KeyCode_LeftAlt && keyState == EKeyState::ePressed)
		//	SG_LOG_DEBUG("Left Alt!");
		//if (keycode == KeyCode_RightAlt && keyState == EKeyState::ePressed)
		//	SG_LOG_DEBUG("Right Alt!");

		if (keyState != EKeyState::ePressed && keyState != EKeyState::eRelease)
			return true;

		auto& io = ImGui::GetIO();
		if (keycode == KeyCode_LeftShift || keycode == KeyCode_RightShift)
			io.AddKeyEvent(ImGuiKey_ModShift, keyState == EKeyState::ePressed);
		if (keycode == KeyCode_LeftControl || keycode == KeyCode_RightControl)
			io.AddKeyEvent(ImGuiKey_ModCtrl, keyState == EKeyState::ePressed);
		if (keycode == KeyCode_LeftAlt || keycode == KeyCode_RightAlt)
			io.AddKeyEvent(ImGuiKey_ModAlt, keyState == EKeyState::ePressed);

		if (keycode >= KeyCode_MouseLeft && keycode <= KeyCode_MouseMiddle)
			io.AddMouseButtonEvent(ToImGuiMouseButton(keycode), (keyState == EKeyState::ePressed));
		else
		{
			io.AddKeyEvent(ToImguiKey(keycode), (keyState == EKeyState::ePressed));
			//io.SetKeyEventNativeData(ToImguiKey(keycode), 65, 30); // To support legacy indexing (<1.87 user code)
		}

		if (io.WantCaptureMouse) // block other event
			return false;
		else
			return true;
	}

	bool ImGuiDriver::OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos)
	{
		ImGuiIO& io = ImGui::GetIO();
		//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		//{
		//	auto& windowRect = pWindow->GetCurrRect();
		//	mousePos[0] += windowRect.left;
		//	mousePos[1] += windowRect.top;
		//}
		io.AddMousePosEvent((float)xPos, (float)yPos);
		mLastValidMousePos = { xPos, yPos };

		if (io.WantCaptureMouse) // block other event
			return false;
		else
			return true;
	}

	bool ImGuiDriver::OnMouseWheelInputUpdate(int direction)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseWheelEvent((float)0, (float)direction);

		if (io.WantCaptureMouse) // block other event
			return false;
		else
			return true;
	}

	bool ImGuiDriver::OnCharInput(Char c) 
	{
		auto& io = ImGui::GetIO();
		io.AddInputCharacter(c);
		return true;
	}

}