#pragma once
#include "3DEngine/Config.h"

#include "System/Logger.h"
#include "System/I3DEngine.h"

namespace SG
{

	class C3DEngine final : public I3DEngine
	{
	public:
		//! Engine core initialization
		SG_3DENGINE_API virtual void OnInit() override;
		//! Engine update per frame
		SG_3DENGINE_API virtual void OnUpdate(float deltaTime) override;
		//! Engine core shutdown
		SG_3DENGINE_API virtual void OnShutdown() override;
	};

}