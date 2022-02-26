#pragma once

#include "Render/GUI/GUIDriver.h"

namespace SG
{

	class ImGuiDriver final : public IGUIDriver
	{
	public:
		virtual bool OnInit() override;
		virtual void OnShutdown() override;

		virtual void OnDraw() override;
	private:
		//ImGuiContext* mpImGuiContext = nullptr;
	};

}