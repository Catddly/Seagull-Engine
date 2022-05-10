#pragma once

#include "Core/Config.h"
#include "System/Logger.h"
#include "Memory/Memory.h"

#include "Reflection/Name.h"
#include "Stl/Utility.h"

#include "eastl/vector.h"
#include "eastl/unordered_map.h"
#include "eastl/functional.h"
#include "eastl/tuple.h"
#include "eastl/string.h"

namespace SG
{

	// forward decoration
	class MessageBusMember;

	typedef eastl::function<void(MessageBusMember* pMember)> TReceivedCallBackFunc;

	namespace Impl
	{
		class EventBase
		{
		public:
			explicit EventBase(const eastl::string& name)
				:mEventName(name)
			{}
			virtual ~EventBase() = default;

			SG_INLINE const char* GetEventName() const
			{
				return mEventName.c_str();
			}
		private:
			string mEventName;
		};

		template <typename TData>
		class Event : public EventBase
		{
		public:
			explicit Event(const eastl::string& name, const TData& event);
			explicit Event(const eastl::string& name, TData&& event);

			void AddEventCallBackFunc(TReceivedCallBackFunc&& func);

			TData& ReceiveEvent(MessageBusMember* pMember);
		private:
			TData mEventData;
			TReceivedCallBackFunc mCallBackFunc = nullptr;
		};

		template <typename TData>
		Event<TData>::Event(const eastl::string& name, TData&& event)
			:EventBase(name), mEventData(eastl::move(event))
		{
		}

		template <typename TData>
		Event<TData>::Event(const eastl::string& name, const TData& event)
			: EventBase(name), mEventData(event)
		{
		}

		template <typename TData>
		TData& Event<TData>::ReceiveEvent(MessageBusMember* pMember)
		{
			if (mCallBackFunc)
				mCallBackFunc(pMember);
			return mEventData;
		}

		template <typename TData>
		void Event<TData>::AddEventCallBackFunc(TReceivedCallBackFunc&& func)
		{
			mCallBackFunc = SG_FWD(func);
		}

		//! Singleton class.
		class MessageBus
		{
		private:
			template <typename T>
			using TListenCallBackFunc = eastl::function<void(T&)>;
		public:
			~MessageBus() = default;

			SG_CORE_API void Join(MessageBusMember* pMember);
			SG_CORE_API void Leave(MessageBusMember* pMember);

			SG_CORE_API void PushEvent(MessageBusMember* pMember, EventBase* pEvent);

			template <typename T>
			T* Listening(const string& eventName, MessageBusMember* pMember);

			SG_CORE_API static MessageBus* GetInstance();
		private:
			friend class System;
			MessageBus() = default;

			void ClearEvents();
		private:
			eastl::vector<MessageBusMember*> mMembers;
			eastl::unordered_map<string, EventBase*> mpEventsMap;
		};

		template <typename T>
		T* MessageBus::Listening(const string& eventName, MessageBusMember* pMember)
		{
			if (mpEventsMap.empty())
				return nullptr;

			auto node = mpEventsMap.find(eventName);
			if (node != mpEventsMap.end())
			{
				auto* pEvent = static_cast<Event<T>*>(node->second);
				return &pEvent->ReceiveEvent(pMember);
			}
			return nullptr;
		}
	}

	class SG_CORE_API MessageBusMember
	{
	private:
		template <typename T>
		using TListenCallBackFunc = eastl::function<void(T&)>;
	public:
		MessageBusMember();
		~MessageBusMember();

		void PushEvent(const string& name);
		void PushEvent(const string& name, TReceivedCallBackFunc&& func);

		template <typename T>
		void PushEvent(const string& name, const T& data);

		template <typename T>
		void PushEvent(const string& name, const T& data, TReceivedCallBackFunc&& func);

		template <typename T>
		void ListenFor(const string& eventName, TListenCallBackFunc<T>&& func);
	};

	template <typename T>
	void MessageBusMember::PushEvent(const string& name, const T& data)
	{
		auto* pEvent = New(Impl::Event<T>, name, data);

		Impl::MessageBus::GetInstance()->PushEvent(this, pEvent);
		//SG_LOG_DEBUG("Event pushed! %s", pEvent->GetEventName());
	}

	template <typename T>
	void MessageBusMember::PushEvent(const string& name, const T& data, TReceivedCallBackFunc&& func)
	{
		auto* pEvent = New(Impl::Event<T>, name, data);
		pEvent->AddEventCallBackFunc(SG_FWD(func));

		Impl::MessageBus::GetInstance()->PushEvent(this, pEvent);
		//SG_LOG_DEBUG("Event pushed! %s", pEvent->GetEventName());
	}

	template <typename T>
	void MessageBusMember::ListenFor(const string& eventName, TListenCallBackFunc<T>&& func)
	{
		T* pData = Impl::MessageBus::GetInstance()->Listening<T>(eventName, this);
		if (pData)
			func(*pData);
	}

}