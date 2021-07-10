#include "StdAfx.h"
#include "Core/System/InputSystem.h"

namespace SG
{

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
		for (auto* e : mpListeners)
		{
			e->OnInputUpdate(EKeyCode::e0, EKeyState::ePressed);
		}
	}

}