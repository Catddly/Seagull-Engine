#include "StdAfx.h"
#include "Core/Private/System/InputSystem.h"

#include "System/ILogger.h"

namespace SG
{

	eastl::vector<eastl::pair<EKeyCode, EKeyState>> CInputSystem::mFrameInputDelta;

	void CInputSystem::RegisterListener(IInputListener* pListener)
	{
		mpListeners.emplace(pListener);
	}

	void CInputSystem::MuteListener(IInputListener* pListener)
	{
		for (auto* e : mpListeners)
		{
			if (e == pListener)
			{
				// Do mute
			}
		}
	}

	void CInputSystem::RemoveListener(IInputListener* pListener)
	{
		for (auto beg = mpListeners.cbegin(); beg != mpListeners.cend(); beg++)
		{
			if (*beg == pListener)
			{
				mpListeners.erase(beg);
				return;
			}
		}
	}

	void CInputSystem::OnUpdate()
	{
		for (auto& input : mFrameInputDelta)
		{
			for (auto* e : mpListeners)
			{
				if (!e->OnInputUpdate(input.first, input.second))
					break;
			}
		}
		mFrameInputDelta.clear();
	}

	void CInputSystem::OnSystemInputEvent(EKeyCode keycode, EKeyState keyState)
	{
		mFrameInputDelta.emplace_back(eastl::make_pair(keycode, keyState));
	}

}