#include "StdAfx.h"
#include "System/Input.h"

#include "System/Logger.h"
#include "Platform/OS.h"

namespace SG
{

	Input::ListenerContainer Input::mpListeners;
	Vector2i Input::mPrevFrameMousePos = {};

	float Input::mKeyHoldThresHold = 0.0f;
	bool Input::mKeyStatusMap[KEYCODE_COUNT] = {};
	eastl::map<EKeyCode, float> Input::mKeyElapsedTimeMap;

	void Input::OnInit()
	{
	}

	void Input::OnShutdown()
	{
	}

	void Input::RegisterListener(EListenerPriority priority, IInputListener* pListener)
	{
		mpListeners.emplace(priority, pListener);
	}

	void Input::MuteListener(IInputListener* pListener)
	{
		for (auto& e : mpListeners)
		{
			if (e.second == pListener)
			{
				// Do mute
			}
		}
	}

	void Input::RemoveListener(IInputListener* pListener)
	{
		for (auto beg = mpListeners.begin(); beg != mpListeners.end(); beg++)
		{
			if (beg->second == pListener)
			{
				mpListeners.erase(beg);
				return;
			}
		}
	}

	void Input::OnUpdate(float deltaTime)
	{
		if (!OperatingSystem::GetMainWindow()->IsMouseCursorInWindow()) // mouse is out of the screen, eliminate all holding event
		{
			for (auto& key : mKeyElapsedTimeMap) // force to release
			{
				for (auto& e : mpListeners)
				{
					if (!e.second->OnKeyInputUpdate(key.first, EKeyState::eRelease))
						break;
				}
				mKeyStatusMap[key.first] = false;
			}
			mKeyElapsedTimeMap.clear();
		}

		for (auto& key : mKeyElapsedTimeMap) // update the timer and pass the hold event
		{
			key.second += deltaTime;
			if (key.second >= mKeyHoldThresHold)
			{
				for (auto& e : mpListeners)
				{
					if (!e.second->OnKeyInputUpdate(key.first, EKeyState::eHold))
						break;
				}
			}
		}
	}

	void Input::OnSystemKeyInputEvent(EKeyCode keycode, bool bPressed)
	{
		if (keycode == KeyCode_Null)
			return;

		if (bPressed)
		{
			if (mKeyElapsedTimeMap.find(keycode) == mKeyElapsedTimeMap.end()) // not exist
			{
				for (auto& e : mpListeners)
				{
					if (!e.second->OnKeyInputUpdate(keycode, EKeyState::ePressed))
						break;
				}
				mKeyElapsedTimeMap[keycode] = 0.0f; // setup current elapsed time
			}
		}
		else // release, no holding event
		{
			for (auto& e : mpListeners)
			{
				if (!e.second->OnKeyInputUpdate(keycode, EKeyState::eRelease))
					break;
			}
			mKeyElapsedTimeMap.erase(keycode);
		}
		mKeyStatusMap[keycode] = bPressed; // update key status map
	}

	void Input::OnSystemMouseMoveInputEvent(int xPos, int yPos)
	{
		Vector2i currFrameMousePos = { xPos, yPos };
		// update mouse moving event
		const int moveDeltaX = currFrameMousePos[0] - mPrevFrameMousePos[0];
		const int moveDeltaY = mPrevFrameMousePos[1] - currFrameMousePos[1];
		for (auto& e : mpListeners)
		{
			if (!e.second->OnMouseMoveInputUpdate(currFrameMousePos[0], currFrameMousePos[1], moveDeltaX, moveDeltaY))
				break;
		}
		mPrevFrameMousePos = currFrameMousePos;
	}

	void Input::OnSystemMouseWheelInputEvent(int direction)
	{
		// update mouse wheeling event
		for (auto& e : mpListeners)
		{
			if (!e.second->OnMouseWheelInputUpdate(direction))
				break;
		}
	}

	void Input::OnSystemCharInput(Char c)
	{
		for (auto& e : mpListeners)
		{
			if (!e.second->OnCharInput(c))
				break;
		}
	}

	void Input::OnSystemWideCharInput(WChar wc)
	{
		for (auto& e : mpListeners)
		{
			if (!e.second->OnWideCharInput(wc))
				break;
		}
	}

	bool Input::IsKeyPressed(EKeyCode keycode)
	{
		return mKeyStatusMap[keycode];
	}

	void Input::SetKeyHoldThreshold(float threshold)
	{
		if (threshold >= 0.0f)
			mKeyHoldThresHold = threshold;
	}

}