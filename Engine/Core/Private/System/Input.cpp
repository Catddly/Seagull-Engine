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

	bool  Input::mKeyStatusMap[KEYCODE_COUNT] = {};
	float Input::mKeyElapsedTimeMap[KEYCODE_COUNT] = {};

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
		for (UInt32 i = 0; i < KEYCODE_COUNT; ++i)
		{
			if (mKeyStatusMap[i]) // if pressed
				mKeyElapsedTimeMap[i] += deltaTime; // update timer
			else
				mKeyElapsedTimeMap[i] = 0.0f;

			if (mKeyElapsedTimeMap[i] >= 500.0f) // hold more than 0.5s
			{
				for (auto* e : mpListeners)
				{
					if (!e->OnKeyInputUpdate(EKeyCode(i), EKeyState::eHold))
						break;
				}
			}
		}

		if (OperatingSystem::GetMainWindow()->IsMouseCursorInWindow())
		{
			int moveDeltaX = mCurrFrameMousePos[0] - mPrevFrameMousePos[0];
			int moveDeltaY = mPrevFrameMousePos[1] - mCurrFrameMousePos[1];

			if (moveDeltaX != 0 || moveDeltaY != 0)
			{
				for (auto* e : mpListeners)
				{
					if (!e->OnMouseMoveInputUpdate(mCurrFrameMousePos[0], mCurrFrameMousePos[1], moveDeltaX, moveDeltaY))
						break;
				}
			}

			mPrevFrameMousePos = mCurrFrameMousePos;

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
	}

	void Input::OnSystemKeyInputEvent(EKeyCode keycode, bool bPressed)
	{
		for (auto* e : mpListeners)
		{
			if (!e->OnKeyInputUpdate(keycode, (bPressed == true) ? EKeyState::ePressed : EKeyState::eRelease))
				break;
		}
		mKeyStatusMap[keycode] = bPressed;
	}

	void Input::OnSystemMouseKeyInputEvent(EKeyCode keycode, bool bPressed)
	{
		for (auto* e : mpListeners)
		{
			if (!e->OnKeyInputUpdate(keycode, (bPressed == true) ? EKeyState::ePressed : EKeyState::eRelease))
				break;
		}
		mKeyStatusMap[keycode] = bPressed;
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
		if (keycode >= 124) // is mouse key
			return mKeyStatusMap[keycode];

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