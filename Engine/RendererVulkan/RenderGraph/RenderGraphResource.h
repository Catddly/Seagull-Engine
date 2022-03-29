#pragma once

#include "Defs/Defs.h"
#include "Memory/Memory.h"
#include "Render/FrameBuffer.h"
#include "Render/ResourceBarriers.h"

#include "RendererVulkan/Config.h"

#include "Stl/vector.h"
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
		RenderGraphResourceBase(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op, const ClearValue& clearValue,
			EResourceBarrier srcStatus, EResourceBarrier dstStatus)
			:mLoadStoreClearOp(op), mClearValue(clearValue),
			mSrcStatus(srcStatus), mDstStatus(dstStatus)
		{
			mpRenderTargets.resize(1);
			mpRenderTargets[0] = pRenderTarget;
		}
		RenderGraphResourceBase(VulkanRenderTarget** ppRenderTarget, UInt32 numRt, LoadStoreClearOp op, const ClearValue& clearValue,
			EResourceBarrier srcStatus, EResourceBarrier dstStatus)
			:mLoadStoreClearOp(op), mClearValue(clearValue),
			mSrcStatus(srcStatus), mDstStatus(dstStatus)
		{
			mpRenderTargets.resize(numRt);
			for (UInt32 i = 0; i < numRt; ++i)
				mpRenderTargets[i] = ppRenderTarget[i];
		}
		virtual ~RenderGraphResourceBase() = default;

		SG_INLINE UInt32 GetNumRenderTarget() const { return static_cast<UInt32>(mpRenderTargets.size()); }
		SG_INLINE VulkanRenderTarget* GetRenderTarget(UInt32 index = 0) const;
		SG_INLINE Size GetDataHash(Size prev = 0, UInt32 index = 0) const;

		SG_INLINE ClearValue GetClearValue() const { return mClearValue; }
		SG_INLINE LoadStoreClearOp GetLoadStoreClearOp() const { return mLoadStoreClearOp; }
		SG_INLINE EResourceBarrier GetSrcStatus() const { return mSrcStatus; }
		SG_INLINE EResourceBarrier GetDstStatus() const { return mDstStatus; }

		SG_INLINE bool operator==(const RenderGraphResourceBase& rhs) const
		{
			if (this->mpRenderTargets.size() != rhs.mpRenderTargets.size())
				return false;
			bool bTheSame = (this->mLoadStoreClearOp == rhs.mLoadStoreClearOp);
			if (!bTheSame)
				return false;
			for (UInt32 i = 0; i < mpRenderTargets.size(); ++i)
				bTheSame &= (this->mpRenderTargets[i] == rhs.mpRenderTargets[i]);
			return bTheSame;
		}
	private:
		vector<VulkanRenderTarget*> mpRenderTargets;
		ClearValue       mClearValue;
		LoadStoreClearOp mLoadStoreClearOp;
		EResourceBarrier mSrcStatus = EResourceBarrier::efUndefined;
		EResourceBarrier mDstStatus = EResourceBarrier::efUndefined;
	};

	template <ERGResourceFlow flow>
	SG_INLINE VulkanRenderTarget* SG::RenderGraphResourceBase<flow>::GetRenderTarget(UInt32 index) const
	{
		SG_ASSERT(index < mpRenderTargets.size() && "Index exceed the boundary!");
		return mpRenderTargets[index];
	}

	template <ERGResourceFlow flow>
	SG_INLINE Size RenderGraphResourceBase<flow>::GetDataHash(Size prev, UInt32 index) const
	{
		SG_ASSERT(index < mpRenderTargets.size() && "Index exceed the boundary!");
		return RGResourceHasher{}(mpRenderTargets[index], mLoadStoreClearOp, prev);
	}

	class RenderGraphInReousrce final : public RenderGraphResourceBase<ERGResourceFlow::eIn>
	{
	public:
		RenderGraphInReousrce(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op, const ClearValue& clearValue,
			EResourceBarrier srcStatus, EResourceBarrier dstStatus)
			: RenderGraphResourceBase(pRenderTarget, op, clearValue, srcStatus, dstStatus)
		{}
		RenderGraphInReousrce(VulkanRenderTarget** ppRenderTarget, UInt32 numRt, LoadStoreClearOp op, const ClearValue& clearValue,
			EResourceBarrier srcStatus, EResourceBarrier dstStatus)
			: RenderGraphResourceBase(ppRenderTarget, numRt, op, clearValue, srcStatus, dstStatus)
		{}
		~RenderGraphInReousrce() = default;
	private:
	};

}