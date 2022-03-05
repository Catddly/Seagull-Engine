#pragma once

#include "Platform/OS.h"
#include "System/Input.h"

#include "Render/GUI/IGUIDriver.h"

namespace SG
{

	class ImGuiDriver final : public IGUIDriver, protected IInputListener
	{
	public:
		ImGuiDriver();
		~ImGuiDriver();

		virtual bool OnInit() override;
		virtual void OnShutdown() override;

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnDraw(Scene* pScene) override;
	private:
		void UpdateMouseData();
		void UpdateCursorData();
	private:
		virtual bool OnKeyInputUpdate(EKeyCode keycode, EKeyState keyState) override;
		virtual bool OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos) override;
		virtual bool OnMouseWheelInputUpdate(int direction) override;
		virtual bool OnCharInput(Char c) override;
	private:
		Vector2i mLastValidMousePos;
	};

}