#pragma once

#include <eastl/queue.h>
#include <eastl/set.h>

#ifdef SG_PLATFORM_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	endif
#endif

namespace SG
{

	enum class ESystemMessage
	{
		eWindowResize = 0,
		eWindowMove,
		eWindowMinimal,
		eTerminate,
	};

	interface SG_CORE_API ISystemMessageListener
	{
		virtual bool OnSystemMessage(ESystemMessage msg) = 0;
	};

	class SystemMessageBus
	{
	public:
		void Update();

		void RegisterListener(ISystemMessageListener* pListener);
		void RemoveListener(ISystemMessageListener* pListener);

		void PushEvent(ESystemMessage msg);
	private:
		eastl::set<ESystemMessage>          mMessages;
		eastl::set<ISystemMessageListener*> mpListeners;
	};

}