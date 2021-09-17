#include "StdAfx.h"
#include "System/ISystemMessage.h"

#include <eastl/type_traits.h>

namespace SG
{

	void SystemMessageBus::Update()
	{
		for (auto beg = mMessages.begin(); beg != mMessages.end(); beg++)
		{
			for (auto* e : mpListeners)
			{
				if (!e->OnSystemMessage(*beg))
					break;
			}
		}
		mMessages.clear();
	}

	void SystemMessageBus::RegisterListener(ISystemMessageListener* pListener)
	{
		mpListeners.emplace(pListener);
	}

	void SystemMessageBus::RemoveListener(ISystemMessageListener* pListener)
	{
		for (auto beg = mpListeners.begin(); beg != mpListeners.end(); beg++)
		{
			if (*beg == pListener)
				mpListeners.erase(beg);
		}
	}

	void SystemMessageBus::PushEvent(ESystemMessage msg)
	{
		mMessages.emplace(msg);
	}

}