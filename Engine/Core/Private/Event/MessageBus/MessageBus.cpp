#include "StdAfx.h"
#include "Event/MessageBus/MessageBus.h"

#include "System/Logger.h"

namespace SG
{

	MessageBusMember::MessageBusMember()
	{
		Impl::MessageBus::GetInstance()->Join(this);
	}

	MessageBusMember::~MessageBusMember()
	{
		Impl::MessageBus::GetInstance()->Leave(this);
	}

	namespace Impl
	{
		void MessageBus::ClearEvents()
		{
			for (auto node : mpEventsMap)
				Memory::Delete(node.second);
			mpEventsMap.clear();
		}

		void MessageBus::Join(MessageBusMember* pMember)
		{
			mMembers.emplace_back(pMember);
			SG_LOG_DEBUG("Member Join: (0x%p)", pMember);
		}

		void MessageBus::Leave(MessageBusMember* pMember)
		{
			for (auto beg = mMembers.begin(); beg != mMembers.end(); ++beg)
			{
				if (*beg == pMember)
				{
					SG_LOG_DEBUG("Member Leave: (0x%p)", *beg);
					mMembers.erase(beg);
					break;
				}
			}
		}

		void MessageBus::PushEvent(MessageBusMember* pMember, EventBase* pEvent)
		{
			string eventName = pEvent->GetEventName();
			auto node = mpEventsMap.find(eventName);
			if (node != mpEventsMap.end())
			{
				auto* pOldEvent = node->second;
				node->second = pEvent;
				Memory::Delete(pOldEvent);
				return;
			}
			mpEventsMap[eventName] = pEvent;
		}

		MessageBus* MessageBus::GetInstance()
		{
			static MessageBus sInstance;
			return &sInstance;
		}
	}

}