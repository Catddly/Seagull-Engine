#pragma once
#include "../Config.h"

#include "Common/System/I3DEngine.h"

namespace SG
{

	class SG_ENGINE_API CEngine final : public I3DEngine
	{
	public:
		//! Engine core initialization
		virtual void OnInit() override;
		//! Engine update per frame
		virtual void OnUpdate() override;
		//! Engine core shutdown
		virtual void OnShutdown() override;

		//! Get the thread id of engine thread
		virtual void GetMainThreadId() const override;
	};

}