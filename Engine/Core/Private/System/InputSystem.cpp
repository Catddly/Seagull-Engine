#include "StdAfx.h"
#include "Core/Private/System/InputSystem.h"

#include "System/ILogger.h"

namespace SG
{

	eastl::vector<eastl::pair<EKeyCode, EKeyState>> InputSystem::msKeyFrameInputDelta;
	eastl::array<bool, 3> InputSystem::msMouseFrameInputDelta;
	eastl::pair<int, int> InputSystem::msMousePosDelta;

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
		static float sTotalTime = 0.0f;

		if (msKeyFrameInputDelta.empty())
		{
			for (auto* e : mpListeners)
			{
				if (!e->OnInputUpdate(KeyCode_Null, EKeyState::eNull, msMousePosDelta.first, msMousePosDelta.second))
					break;
			}
		}
		else
		{
			for (auto& input : msKeyFrameInputDelta)
			{
				for (auto* e : mpListeners)
				{
					if (!e->OnInputUpdate(input.first, input.second, msMousePosDelta.first, msMousePosDelta.second))
						break;
				}
			}
			msKeyFrameInputDelta.clear();
		}

		static bool bGoGoGO = false;
		if (sTotalTime >= 300.0f || bGoGoGO)
		{
			bGoGoGO = true;
			for (Size i = 0; i < msMouseFrameInputDelta.size(); ++i)
			{
				if (msMouseFrameInputDelta[i])
				{
					for (auto* e : mpListeners)
					{
						if (!e->OnInputUpdate(EKeyCode(i + 124), EKeyState::eHold, msMousePosDelta.first, msMousePosDelta.second))
							break;
					}
				}
			}
			sTotalTime = 0.0f;
		}

		if (msMouseFrameInputDelta[0] || msMouseFrameInputDelta[1] || msMouseFrameInputDelta[2])
			sTotalTime += deltaTime;
		if (!msMouseFrameInputDelta[0] && !msMouseFrameInputDelta[1] && !msMouseFrameInputDelta[2])
		{
			bGoGoGO = false;
			sTotalTime = 0.0f;
		}
	}

	void InputSystem::OnSystemKeyInputEvent(EKeyCode keycode, EKeyState keyState)
	{
		msKeyFrameInputDelta.emplace_back(eastl::make_pair(keycode, keyState));
	}

	void InputSystem::OnSystemMouseInputEvent(EKeyCode keycode, EKeyState keyState, int xPos, int yPos)
	{
		if (keyState == EKeyState::ePressed)
		{
			msKeyFrameInputDelta.emplace_back(eastl::make_pair(keycode, keyState));
			msMouseFrameInputDelta[keycode - 124] = true;
		}
		if (keyState == EKeyState::eRelease)
		{
			msKeyFrameInputDelta.emplace_back(eastl::make_pair(keycode, keyState));
			msMouseFrameInputDelta[keycode - 124] = false;
		}

		if (xPos == -1 || yPos == -1) // no update mouse position
			return;
		msMousePosDelta = { xPos, yPos };
	}

}