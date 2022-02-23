#pragma once

#include "Defs/Defs.h"
#include "Render/FrameBuffer.h"

#include "RendererVulkan/Config.h"

#include "Stl/Hash.h"

namespace SG
{

	enum class ERGResourceType
	{
		ePermenant = 0,
		eNormal,
		eTransient,
	};

	enum class ERGResourceFlow
	{
		eIn = 0,
		eOut,
		eDead,
	};

	class VulkanRenderTarget;

	//! Split the hash function from RenderGraphResourceBase,
	//! then we only get one function instance but not as same as the number of ERGResourceFlow.
	struct RGResourceHasher
	{
		Size operator()(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op, Size prev = 0);
	};

	template <ERGResourceFlow flow>
	class RenderGraphResourceBase
	{
	public:
		RenderGraphResourceBase(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op)
			:mpRenderTarget(pRenderTarget), mLoadStoreClearOp(op)
		{}
		virtual ~RenderGraphResourceBase() = default;

		SG_INLINE Size GetDataHash(Size prev = 0) const;

		SG_INLINE bool operator==(const RenderGraphResourceBase& rhs) const
		{
			return (this->mpRenderTarget == rhs.mpRenderTarget) &&
				(this->mLoadStoreClearOp == rhs.mLoadStoreClearOp);
		}
	private:
		friend class RenderGraph;

		VulkanRenderTarget* mpRenderTarget;
		LoadStoreClearOp    mLoadStoreClearOp;
	};

	template <ERGResourceFlow flow>
	SG_INLINE Size RenderGraphResourceBase<flow>::GetDataHash(Size prev) const
	{
		return RGResourceHasher{}(mpRenderTarget, mLoadStoreClearOp, prev);
	}

	class RenderGraphInReousrce final : public RenderGraphResourceBase<ERGResourceFlow::eIn>
	{
	public:
		RenderGraphInReousrce(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op)
			: RenderGraphResourceBase(pRenderTarget, op)
		{}
		~RenderGraphInReousrce() = default;
	private:
	};

	class RenderGraphOutReousrce final : public RenderGraphResourceBase<ERGResourceFlow::eOut>
	{
	public:
		RenderGraphOutReousrce(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op)
			: RenderGraphResourceBase(pRenderTarget, op)
		{}
		~RenderGraphOutReousrce() = default;
	};

}