#pragma once

#include "Common/System/IInput.h"

#include "Common/Stl/vector.h"

#include <EASTL/set.h>
#include <EASTL/utility.h>

// forward declaration for windows' system Proc function
#ifdef SG_PLATFORM_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	endif
	static LRESULT CALLBACK _WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

namespace SG
{

	class CInputSystem : public IInputSystem
	{
	public:
		CInputSystem() = default;
		~CInputSystem() = default;

		virtual void RegisterListener(IInputListener* pListener) override;
		virtual void MuteListener(IInputListener* pListener) override;
		virtual void RemoveListener(IInputListener* pListener) override;

		virtual void OnUpdate() override;
	private:
#ifdef SG_PLATFORM_WINDOWS
		friend static LRESULT CALLBACK _WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
		static void OnSystemInputEvent(EKeyCode keycode, EKeyState keyState);
	private:
		eastl::set<IInputListener*> mpListeners;
		static eastl::vector<eastl::pair<EKeyCode, EKeyState>> mFrameInputDelta;
	};

}