#pragma once

#include "System/IInput.h"

#include "Stl/vector.h"

#include <EASTL/set.h>
#include <EASTL/utility.h>
#include <EASTL/array.h>

// forward declaration for windows' system Proc function
#ifdef SG_PLATFORM_WINDOWS
	static LRESULT CALLBACK _WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

namespace SG
{

	class InputSystem : public IInputSystem
	{
	public:
		InputSystem() = default;
		~InputSystem() = default;

		virtual void OnInit() override {}
		virtual void OnShutdown() override {}

		virtual void RegisterListener(IInputListener* pListener) override;
		virtual void MuteListener(IInputListener* pListener) override;
		virtual void RemoveListener(IInputListener* pListener) override;

		virtual void OnUpdate(float deltaTime) override;

		virtual const char* GetRegisterName() const override { return "InputSystem"; }
	private:
#ifdef SG_PLATFORM_WINDOWS
		friend static LRESULT CALLBACK _WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
		static void OnSystemKeyInputEvent(EKeyCode keycode, EKeyState keyState);
		static void OnSystemMouseInputEvent(EKeyCode keycode, EKeyState keyState, int xPos, int yPos);
	private:
		eastl::set<IInputListener*> mpListeners;
		static eastl::vector<eastl::pair<EKeyCode, EKeyState>> msKeyFrameInputDelta;
		static eastl::array<bool, 3> msMouseFrameInputDelta;
		static eastl::pair<int, int> msMousePosDelta;
	};

}