#include "StdAfx.h"
#include "System/Input.h"

#include "System/Logger.h"
#include "Platform/OS.h"
#include "Profile/Profile.h"

namespace SG
{

	Input::ListenerContainer Input::mpListeners;
	Vector2i Input::mPrevFrameMousePos = {};

	float Input::mKeyHoldThresHold = 0.05f;
	eastl::map<EKeyCode, float> Input::mKeyElapsedTimeMap;
	bool Input::mKeyStatusMap[KEYCODE_COUNT] = {};
	bool Input::mbPrevFocusStatus = false;
	bool Input::mbForceReleaseAllEvent = false;

	void Input::OnInit()
	{
		SG_PROFILE_FUNCTION();
	}

	void Input::OnShutdown()
	{
		SG_PROFILE_FUNCTION();
	}

	void Input::RegisterListener(EListenerPriority priority, IInputListener* pListener)
	{
		SG_PROFILE_FUNCTION();

		mpListeners.emplace(priority, pListener);
	}

	void Input::MuteListener(IInputListener* pListener)
	{
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

		Window* pWindow = OperatingSystem::GetMainWindow();

		// window is lost focus, some key status had become invalid.
		// reset all the key status so that the window will not receive wrong input event.
		bool currentFocusStatus = pWindow->IsFocus();
		if (!pWindow->IsMouseCursorInWindow() || (!currentFocusStatus && mbPrevFocusStatus)) // mouse is out of the screen, eliminate all holding event
			ForceReleaseAllEvent();
		mbPrevFocusStatus = currentFocusStatus;

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

		if (mbForceReleaseAllEvent)
		{
			ReleaseAllEvent();
			mbForceReleaseAllEvent = false;
		}
	}

	void Input::ReleaseAllEvent()
	{
		for (auto& key : mKeyElapsedTimeMap) // force to release
		{
			for (auto& e : mpListeners)
			{
				if (!e.second->OnKeyInputUpdate(key.first, EKeyState::eRelease))
					break;
			}
		}
		mKeyElapsedTimeMap.clear();
		memset(mKeyStatusMap, 0, sizeof(mKeyStatusMap));
	}

	void Input::OnSystemKeyInputEvent(EKeyCode keycode, bool bPressed)
	{
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

		// update mouse wheeling event
		for (auto& e : mpListeners)
		{
			if (!e.second->OnMouseWheelInputUpdate(direction))
				break;
		}
	}

	void Input::OnSystemCharInput(Char c)
	{
		SG_PROFILE_FUNCTION();

		for (auto& e : mpListeners)
		{
			if (!e.second->OnCharInput(c))
				break;
		}
	}

	void Input::OnSystemWideCharInput(WChar wc)
	{
		SG_PROFILE_FUNCTION();

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

	void Input::ForceReleaseAllEvent()
	{
		// deferred the release operation, so the mKeyElapsedTimeMap will not be unsafe.
		mbForceReleaseAllEvent = true;
	}

}