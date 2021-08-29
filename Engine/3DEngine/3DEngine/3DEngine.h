#pragma once
#include "3DEngine/Config.h"

#include "System/ILogger.h"
#include "System/I3DEngine.h"

namespace SG
{

	class C3DEngine final : public I3DEngine
	{
	public:
		//! Engine core initialization
		SG_ENGINE_API virtual void OnInit() override;
		//! Engine update per frame
		SG_ENGINE_API virtual void OnUpdate() override;
		//! Engine core shutdown
		SG_ENGINE_API virtual void OnShutdown() override;
	};

}