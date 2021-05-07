#pragma once
#include "Config.h"

#include "Common/System/IProcess.h"

namespace SG
{

	class SG_ENGINE_API SEngine final : public IProcess
	{
	public:
		//! Engine core initialization
		virtual void OnInit() override;
		//! Engine update per frame
		virtual void OnUpdate() override;
		//! Engine core shutdown
		virtual void OnExit() override;
	};

}