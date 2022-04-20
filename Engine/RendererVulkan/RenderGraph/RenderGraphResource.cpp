#include "StdAfx.h"
#include "RenderGraphResource.h"

#include "RendererVulkan/Backend/VulkanTexture.h"

namespace SG
{

	Size RGRenderPassHasher::operator()(VulkanRenderTarget* pRenderTarget, LoadStoreClearOp op, Size prev)
	{
		Size hash = prev;
		if (!pRenderTarget->IsDepth())
		{
			UInt32 address[3] = {
				static_cast<UInt32>(pRenderTarget->GetFormat()),
				static_cast<UInt32>(pRenderTarget->GetSample()),
				static_cast<UInt32>(op.loadOp)
			};
			return HashMemory32Array(address, 3, hash);
		}
		else
		{
			UInt32 address[4] = {
				static_cast<UInt32>(pRenderTarget->GetFormat()),
				static_cast<UInt32>(pRenderTarget->GetSample()),
				static_cast<UInt32>(op.loadOp),
				static_cast<UInt32>(op.stencilLoadOp)
			};
			return HashMemory32Array(address, 4, hash);
		}
		return hash;
	}

	Size RGFrameBufferHasher::operator()(VulkanRenderTarget* pRenderTarget, Size prev)
	{
		Size hash = prev;
		UInt32 address[3] = {
			pRenderTarget->GetID(),
			pRenderTarget->GetNumMipmap(),
			pRenderTarget->GetNumArray()
		};
		return HashMemory32Array(address, 3, hash);
	}

}