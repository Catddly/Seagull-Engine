#pragma once

#include "Platform/OS.h"
#include "System/Input.h"

#include "Render/GUI/GUIDriver.h"

namespace SG
{

	class ImGuiDriver final : public IGUIDriver, protected IInputListener
	{
	public:
		ImGuiDriver();
		~ImGuiDriver();

		virtual bool OnInit() override;
		virtual void OnShutdown() override;

		virtual void OnUpdate() override;
	private:
		void UpdateMouseData();
		void UpdateCursorData();

		virtual bool OnKeyInputUpdate(EKeyCode keycode, EKeyState keyState) override;
		virtual bool OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos) override;
		virtual bool OnMouseWheelInputUpdate(int direction) override;
	private:
		Vector2i mLastValidMousePos;
	};

}