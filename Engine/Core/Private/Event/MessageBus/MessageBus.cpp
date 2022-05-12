#include "StdAfx.h"
#include "Event/MessageBus/MessageBus.h"

#include "System/Logger.h"

namespace SG
{

	MessageBusMember::MessageBusMember()
	{
		SG_PROFILE_FUNCTION();

		Impl::MessageBus::GetInstance()->Join(this);
	}

	MessageBusMember::~MessageBusMember()
	{
		SG_PROFILE_FUNCTION();

		Impl::MessageBus::GetInstance()->Leave(this);
	}

	void MessageBusMember::PushEvent(const string& name)
	{
		SG_PROFILE_FUNCTION();

		auto* pEvent = New(Impl::Event<bool>, name, false);

		Impl::MessageBus::GetInstance()->PushEventWithoutArgs(this, pEvent);
	}

	void MessageBusMember::PushEvent(const string& name, TReceivedCallBackFunc&& func)
	{
		SG_PROFILE_FUNCTION();

		auto* pEvent = New(Impl::Event<bool>, name, false);
		pEvent->AddEventCallBackFunc(SG_FWD(func));

		Impl::MessageBus::GetInstance()->PushEventWithoutArgs(this, pEvent);
	}

	void MessageBusMember::ListenFor(const string& eventName, TListenCallBackFuncWithoutArg&& func)
	{
		SG_PROFILE_FUNCTION();

		bool res = Impl::MessageBus::GetInstance()->Listening(eventName, this);
		if (res)
			func();
	}

	namespace Impl
	{
		void MessageBus::ClearEvents()
		{
			SG_PROFILE_FUNCTION();

			for (auto node : mpEventsWithoutArgsMap)
				Delete(node.second);
			mpEventsWithoutArgsMap.clear();
			for (auto node : mpEventsMap)
				Delete(node.second);
			mpEventsMap.clear();
		}

		void MessageBus::Join(MessageBusMember* pMember)
		{
			SG_PROFILE_FUNCTION();

			mMembers.emplace_back(pMember);
			SG_LOG_DEBUG("Member Join: (0x%p)", pMember);
		}

		void MessageBus::Leave(MessageBusMember* pMember)
		{
			SG_PROFILE_FUNCTION();

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
			SG_PROFILE_FUNCTION();

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

		void MessageBus::PushEventWithoutArgs(MessageBusMember* pMember, EventBase* pEvent)
		{
			SG_PROFILE_FUNCTION();

			string eventName = pEvent->GetEventName();
			auto node = mpEventsWithoutArgsMap.find(eventName);
			if (node != mpEventsWithoutArgsMap.end())
			{
				auto* pOldEvent = node->second;
				node->second = pEvent;
				Delete(pOldEvent);
				return;
			}
			mpEventsWithoutArgsMap[eventName] = pEvent;
		}

		bool MessageBus::Listening(const string& eventName, MessageBusMember* pMember)
		{
			SG_PROFILE_FUNCTION();

			if (mpEventsWithoutArgsMap.empty())
				return false;

			auto node = mpEventsWithoutArgsMap.find(eventName);
			if (node != mpEventsWithoutArgsMap.end())
			{
				auto* pEvent = static_cast<Event<bool>*>(node->second);
				pEvent->ReceiveEvent(pMember);
				return true;
			}
			return false;
		}

		MessageBus* MessageBus::GetInstance()
		{
			static MessageBus sInstance;
			return &sInstance;
		}
	}

}