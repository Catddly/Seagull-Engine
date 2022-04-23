#include "StdAfx.h"
#include "System/System.h"

#include "Profile/Profile.h"

#include <eastl/type_traits.h>

namespace SG
{

	void SystemMessageManager::Update()
	{
		SG_PROFILE_FUNCTION();

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

	void SystemMessageManager::RegisterListener(ISystemMessageListener* pListener)
	{
		mpListeners.emplace(pListener);
	}

	void SystemMessageManager::RemoveListener(ISystemMessageListener* pListener)
	{
		auto pRemove = mpListeners.begin();
		for (auto beg = mpListeners.begin(); beg != mpListeners.end(); ++beg)
		{
			if (*beg == pListener)
			{
				pRemove = beg;
				break;
			}
		}
		mpListeners.erase(pRemove);
	}

	void SystemMessageManager::PushEvent(ESystemMessage msg)
	{
		mMessages.emplace(msg);
	}

}