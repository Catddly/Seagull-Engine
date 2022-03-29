#pragma once

#include "Defs/Defs.h"
#include "Render/FrameBuffer.h"
#include "Render/ResourceBarriers.h"

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
		RenderGraphResourceBase(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op, const ClearValue& clearValue)
			:mppRenderTarget(&pRenderTarget), mLoadStoreClearOp(op), mClearValue(clearValue), mNumRenderTarget(1),
			mSrcStatus(EResourceBarrier::efUndefined), mDstStatus(EResourceBarrier::efUndefined)
		{}
		RenderGraphResourceBase(VulkanRenderTarget** ppRenderTarget, UInt32 numRt, LoadStoreClearOp op, const ClearValue& clearValue)
			:mppRenderTarget(ppRenderTarget), mLoadStoreClearOp(op), mClearValue(clearValue), mNumRenderTarget(numRt),
			mSrcStatus(EResourceBarrier::efUndefined), mDstStatus(EResourceBarrier::efUndefined)
		{}
		RenderGraphResourceBase(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op, const ClearValue& clearValue,
			EResourceBarrier srcStatus, EResourceBarrier dstStatus)
			:mppRenderTarget(&pRenderTarget), mLoadStoreClearOp(op), mClearValue(clearValue), mNumRenderTarget(1),
			mSrcStatus(srcStatus), mDstStatus(dstStatus)
		{}
		virtual ~RenderGraphResourceBase() = default;

		SG_INLINE UInt32 GetNumRenderTarget() const { return mNumRenderTarget; }
		SG_INLINE VulkanRenderTarget* GetRenderTarget(UInt32 index = 0) const;
		SG_INLINE Size GetDataHash(Size prev = 0, UInt32 index = 0) const;

		SG_INLINE ClearValue GetClearValue() const { return mClearValue; }
		SG_INLINE LoadStoreClearOp GetLoadStoreClearOp() const { return mLoadStoreClearOp; }
		SG_INLINE EResourceBarrier GetSrcStatus() const { return mSrcStatus; }
		SG_INLINE EResourceBarrier GetDstStatus() const { return mDstStatus; }

		SG_INLINE bool operator==(const RenderGraphResourceBase& rhs) const
		{
			if (this->mNumRenderTarget != rhs.mNumRenderTarget)
				return false;
			bool bTheSame = (this->mLoadStoreClearOp == rhs.mLoadStoreClearOp);
			if (!bTheSame)
				return false;
			for (UInt32 i = 0; i < mNumRenderTarget; ++i)
				bTheSame &= (this->mppRenderTarget[i] == rhs.mppRenderTarget[i]);
			return bTheSame;
		}
	private:
		VulkanRenderTarget** mppRenderTarget;
		UInt32               mNumRenderTarget;
		ClearValue           mClearValue;
		LoadStoreClearOp     mLoadStoreClearOp;
		EResourceBarrier     mSrcStatus;
		EResourceBarrier     mDstStatus;
	};

	template <ERGResourceFlow flow>
	SG_INLINE VulkanRenderTarget* SG::RenderGraphResourceBase<flow>::GetRenderTarget(UInt32 index) const
	{
		SG_ASSERT(index < mNumRenderTarget && "Index exceed the boundary!");
		return mppRenderTarget[index];
	}

	template <ERGResourceFlow flow>
	SG_INLINE Size RenderGraphResourceBase<flow>::GetDataHash(Size prev, UInt32 index) const
	{
		SG_ASSERT(index < mNumRenderTarget && "Index exceed the boundary!");
		return RGResourceHasher{}(mppRenderTarget[index], mLoadStoreClearOp, prev);
	}

	class RenderGraphInReousrce final : public RenderGraphResourceBase<ERGResourceFlow::eIn>
	{
	public:
		RenderGraphInReousrce(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op, const ClearValue& clearValue)
			: RenderGraphResourceBase(pRenderTarget, op, clearValue)
		{}
		RenderGraphInReousrce(VulkanRenderTarget** ppRenderTarget, UInt32 numRt, LoadStoreClearOp op, const ClearValue& clearValue)
			: RenderGraphResourceBase(ppRenderTarget, numRt, op, clearValue)
		{}
		RenderGraphInReousrce(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op, const ClearValue& clearValue,
			EResourceBarrier srcStatus, EResourceBarrier dstStatus)
			: RenderGraphResourceBase(pRenderTarget, op, clearValue, srcStatus, dstStatus)
		{}
		~RenderGraphInReousrce() = default;
	private:
	};

}