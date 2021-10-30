#include "StdAfx.h"
#include "System/Input.h"

#include "System/Logger.h"

namespace SG
{

	eastl::set<IInputListener*> Input::mpListeners;
	eastl::vector<eastl::pair<EKeyCode, EKeyState>> Input::msKeyFrameInputDelta;
	eastl::array<bool, 3> Input::msMouseFrameInputDelta;
	eastl::array<bool, 3> Input::msMousePrevFrameInputDelta;

	void Input::OnInit()
	{
		for (auto& e : msMousePrevFrameInputDelta)
			e = false;
	}

	void Input::OnShutdown()
	{
	}

	void Input::RegisterListener(IInputListener* pListener)
	{
		mpListeners.emplace(pListener);
	}

	void Input::MuteListener(IInputListener* pListener)
	{
		for (auto* e : mpListeners)
		{
			if (e == pListener)
			{
				// Do mute
			}
		}
	}

	void Input::RemoveListener(IInputListener* pListener)
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

	void Input::OnUpdate(float deltaTime)
	{
		if (msKeyFrameInputDelta.empty())
		{
			for (auto* e : mpListeners)
			{
				if (!e->OnKeyInputUpdate(KeyCode_Null, EKeyState::eNull))
					break;
			}
		}
		else
		{
			for (auto& input : msKeyFrameInputDelta)
			{
				for (auto* e : mpListeners)
				{
					if (!e->OnKeyInputUpdate(input.first, input.second))
						break;
				}
			}
			msKeyFrameInputDelta.clear();
		}

		for (Size i = 0; i < msMouseFrameInputDelta.size(); ++i)
		{
			if (msMouseFrameInputDelta[i] && msMousePrevFrameInputDelta[i]) // pressed once but not release
			{
				for (auto* e : mpListeners)
				{
					if (!e->OnKeyInputUpdate(EKeyCode(i + 124), EKeyState::eHold)) // mouse button holding event
						break;
				}
			}
			msMousePrevFrameInputDelta[i] = msMouseFrameInputDelta[i];
		}
	}

	void Input::OnSystemKeyInputEvent(EKeyCode keycode, EKeyState keyState)
	{
		msKeyFrameInputDelta.emplace_back(eastl::make_pair(keycode, keyState));
	}

	void Input::OnSystemMouseKeyInputEvent(EKeyCode keycode, EKeyState keyState)
	{
		if (keyState == EKeyState::ePressed)
		{
			msKeyFrameInputDelta.emplace_back(eastl::make_pair(keycode, keyState));
			msMouseFrameInputDelta[keycode - 124] = true;
		}
		else if (keyState == EKeyState::eRelease)
		{
			msKeyFrameInputDelta.emplace_back(eastl::make_pair(keycode, keyState));
			msMouseFrameInputDelta[keycode - 124] = false;
		}
	}

	void Input::OnSystemMouseMoveInputEvent(int xPos, int yPos)
	{
		static int sPrevMousePosX = xPos;
		static int sPrevMousePosY = yPos;

		int moveDeltaX = xPos - sPrevMousePosX;
		int moveDeltaY = sPrevMousePosY - yPos;

		if (moveDeltaX != 0 || moveDeltaY != 0)
		{
			for (auto* e : mpListeners)
			{
				if (!e->OnMouseMoveInputUpdate(xPos, yPos, moveDeltaX, moveDeltaY))
					break;
			}
		}

		sPrevMousePosX = xPos;
		sPrevMousePosY = yPos;
	}

	void Input::OnSystemMouseWheelInputEvent(int direction)
	{
		for (auto* e : mpListeners)
		{
			if (!e->OnMouseWheelInputUpdate(direction))
				break;
		}
	}

	bool Input::IsKeyPressed(EKeyCode keycode)
	{
		if (keycode >= 124) // is mouse key
			return msMouseFrameInputDelta[keycode - 124];

		SHORT ret = ::GetAsyncKeyState(gKeyCodeToPlatformMap[keycode]);
		// Get the MSB
#if   SG_BIG_ENDIAN
		return ret & 0x80;
#elif SG_LITTLE_ENDIAN
		return ret & 0x01;
#else
#	error Unknown endian (Maybe the middle-endian)
#endif
	}

}