#include "StdAfx.h"
#include "System/ISystemMessage.h"

#include <eastl/type_traits.h>

namespace SG
{

	void SystemMessageBus::Update()
	{
		while (mMessages.size() != 0)
		{
			ESystemMessage msg = mMessages.front();
			for (auto* e : mpListeners)
			{
				if (!e->OnSystemMessage(msg))
					break;
			}
			mMessages.pop();
		}
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
		mMessages.push(msg);
	}

}