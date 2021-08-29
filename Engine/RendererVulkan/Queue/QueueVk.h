#pragma once

#include "Render/Queue.h"
#include "Base/BasicTypes.h"

#include <vulkan/vulkan_core.h>

#include <EASTL/optional.h>

namespace SG
{

	class RendererVk;
	class QueueVk final : public Queue
	{
	public:
		QueueVk(EQueueType type, EQueuePriority priority, RendererVk* pRenderer);
		~QueueVk();

		virtual EQueueType GetQueueType() const override;
		virtual EQueuePriority GetPriority() const override;
		virtual bool       IsValid() const override;
		virtual UInt32     GetQueueIndex() const override; // maybe we don't want this

		virtual QueueHandle GetNativeHandle() override;

		bool operator==(const QueueVk& rhs) const;
		bool operator!=(const QueueVk& rhs) const;
	private:
		RendererVk* mpRenderer = nullptr;
		eastl::optional<UInt32> mIndex;
		EQueueType mType = EQueueType::eNull;
		EQueuePriority mPriority = EQueuePriority::eNormal;
		VkQueue mHandle;
	};

}