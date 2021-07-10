#pragma once

#include "Common/System/IInput.h"

#include <EASTL/set.h>

namespace SG
{

	class CInputSystem : public IInputSystem
	{
	public:
		CInputSystem() = default;
		~CInputSystem() = default;

		virtual void RegisterListener(IInputListener* pListener) override;
		virtual void MuteListener(IInputListener* pListener) override;
		virtual void RemoveListener(IInputListener* pListener) override;

		virtual void OnUpdate() override;
	private:
		eastl::set<IInputListener*> mpListeners;
	};

}