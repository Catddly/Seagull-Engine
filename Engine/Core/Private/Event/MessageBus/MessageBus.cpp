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

	void MessageBusMember::PushEvent(const string& name)
	{
		auto* pEvent = New(Impl::Event<bool>, name, false);

		Impl::MessageBus::GetInstance()->PushEvent(this, pEvent);
	}

	void MessageBusMember::PushEvent(const string& name, TReceivedCallBackFunc&& func)
	{
		auto* pEvent = New(Impl::Event<bool>, name, false);
		pEvent->AddEventCallBackFunc(SG_FWD(func));

		Impl::MessageBus::GetInstance()->PushEvent(this, pEvent);
	}

	namespace Impl
	{
		void MessageBus::ClearEvents()
		{
			for (auto node : mpEventsMap)
				Delete(node.second);
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
				Delete(pOldEvent);
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