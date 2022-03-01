#include "StdAfx.h"
#include "System/Input.h"

#include "System/Logger.h"
#include "Platform/OS.h"

namespace SG
{

	eastl::set<IInputListener*> Input::mpListeners;
	Vector2i Input::mCurrFrameMousePos;
	Vector2i Input::mPrevFrameMousePos;

	int Input::mCurrFrameWheelDirection = 0;

	bool Input::mKeyStatusMap[KEYCODE_COUNT] = {};
	eastl::map<EKeyCode, float> Input::mKeyElapsedTimeMap;

	void Input::OnInit()
	{
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
		if (!OperatingSystem::GetMainWindow()->IsMouseCursorInWindow()) // mouse is out of the screen, eliminate all holding event
		{
			for (auto& key : mKeyElapsedTimeMap) // force to release
			{
				for (auto* e : mpListeners)
				{
					if (!e->OnKeyInputUpdate(key.first, EKeyState::eRelease))
						break;
				}
				mKeyStatusMap[key.first] = false;
			}
			mKeyElapsedTimeMap.clear();
		}

		for (auto& key : mKeyElapsedTimeMap) // update the timer and pass the hold event
		{
			key.second += deltaTime;
			if (key.second >= 0.5f) // TODO: expose this param to HoldTimeThreshold
			{
				for (auto* e : mpListeners)
				{
					if (!e->OnKeyInputUpdate(key.first, EKeyState::eHold))
						break;
				}
			}
		}

		// update mouse moving event
		const int moveDeltaX = mCurrFrameMousePos[0] - mPrevFrameMousePos[0];
		const int moveDeltaY = mPrevFrameMousePos[1] - mCurrFrameMousePos[1];
		if (moveDeltaX != 0 || moveDeltaY != 0)
		{
			for (auto* e : mpListeners)
			{
				if (!e->OnMouseMoveInputUpdate(mCurrFrameMousePos[0], mCurrFrameMousePos[1], moveDeltaX, moveDeltaY))
					break;
			}
		}
		mPrevFrameMousePos = mCurrFrameMousePos;

		// update mouse wheeling event
		if (mCurrFrameWheelDirection != 0)
		{
			for (auto* e : mpListeners)
			{
				if (!e->OnMouseWheelInputUpdate(mCurrFrameWheelDirection))
					break;
			}
			mCurrFrameWheelDirection = 0;
		}
	}

	void Input::OnSystemKeyInputEvent(EKeyCode keycode, bool bPressed)
	{
		if (bPressed)
		{
			if (mKeyElapsedTimeMap.find(keycode) == mKeyElapsedTimeMap.end()) // not exist
			{
				for (auto* e : mpListeners)
				{
					if (!e->OnKeyInputUpdate(keycode, EKeyState::ePressed))
						break;
				}
				mKeyElapsedTimeMap[keycode] = 0.0f; // setup current elapsed time
			}
		}
		else // release, no holding event
		{
			for (auto* e : mpListeners)
			{
				if (!e->OnKeyInputUpdate(keycode, EKeyState::eRelease))
					break;
			}
			mKeyElapsedTimeMap.erase(keycode);
		}
		mKeyStatusMap[keycode] = bPressed; // update key status map
	}

	void Input::OnSystemMouseMoveInputEvent(int xPos, int yPos)
	{
		mCurrFrameMousePos = { xPos, yPos };
	}

	void Input::OnSystemMouseWheelInputEvent(int direction)
	{
		mCurrFrameWheelDirection = direction;
	}

	bool Input::IsKeyPressed(EKeyCode keycode)
	{
		return mKeyStatusMap[keycode];
	}

}