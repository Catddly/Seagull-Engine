#pragma once

#include "Common/Platform/IOperatingSystem.h"

namespace SG
{

	struct CWindowsMouseOp : public IMouseOp
	{
		virtual Vec2 GetMousePosAbsolute() const override;
		virtual Vec2 GetMousePosRelative(SWindow* const pWindow) const override;
	};

}