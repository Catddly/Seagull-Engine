#include "StdAfx.h"
#include "Core/Private/System/InputSystem.h"

#include "System/ILogger.h"

namespace SG
{

	eastl::vector<eastl::pair<EKeyCode, EKeyState>> InputSystem::mFrameInputDelta;

	void InputSystem::RegisterListener(IInputListener* pListener)
	{
		mpListeners.emplace(pListener);
	}

	void InputSystem::MuteListener(IInputListener* pListener)
	{
		for (auto* e : mpListeners)
		{
			if (e == pListener)
			{
				// Do mute
			}
		}
	}

	void InputSystem::RemoveListener(IInputListener* pListener)
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

	void InputSystem::OnUpdate(float deltaTime)
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

	void InputSystem::OnSystemInputEvent(EKeyCode keycode, EKeyState keyState)
	{
		mFrameInputDelta.emplace_back(eastl::make_pair(keycode, keyState));
	}

}