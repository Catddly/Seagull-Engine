#include "StdAfx.h"
#include "RendererVulkan/Renderer/DebugRenderer.h"

#include "Profile/Profile.h"

namespace SG
{

	VulkanContext* DebugRenderer::mpContext = nullptr;
	VulkanCommandBuffer* DebugRenderer::mpCmdBuf = nullptr;
	UInt32 DebugRenderer::mCurrFrameIndex = 0;

	//eastl::fixed_map<EMeshPass, eastl::vector<SG::IndirectDrawCall>, (UInt32)EMeshPass> DebugRenderer::mDrawCallMap;
	bool DebugRenderer::mbRendererInit = false;
	bool DebugRenderer::mbBeginDraw = false;
	bool DebugRenderer::mbDrawCallReady = false;

	void DebugRenderer::OnInit(VulkanContext& context)
	{
		SG_PROFILE_FUNCTION();

		mpContext = &context;
		mbRendererInit = true;
	}

	void DebugRenderer::OnShutdown()
	{
		SG_PROFILE_FUNCTION();

		//mDrawCallMap.clear();

		mpContext = nullptr;
		mbRendererInit = false;
	}

	void DebugRenderer::Begin(DrawInfo& drawInfo)
	{
		SG_PROFILE_FUNCTION();

		if (mbBeginDraw)
		{
			SG_LOG_ERROR("Did you forget to call End() after drawing?");
			return;
		}
		if (!mbDrawCallReady)
		{
			SG_LOG_WARN("Please call CollectRenderData() before your drawing!");
			return;
		}

		mbBeginDraw = true;
		mpCmdBuf = drawInfo.pCmd;
		mCurrFrameIndex = drawInfo.frameIndex;
	}

	void DebugRenderer::End()
	{
		SG_PROFILE_FUNCTION();

		if (!mbBeginDraw)
		{
			SG_LOG_ERROR("Did you forget to call Begin() before drawing?");
			return;
		}

		mbBeginDraw = false;
		mpCmdBuf = nullptr;
		mCurrFrameIndex = UInt32(-1);
	}

	void DebugRenderer::Draw(EMeshPass meshPass)
	{

	}

	void DebugRenderer::BindMesh(const DrawMesh& drawMesh)
	{

	}

	void DebugRenderer::BindMaterial(const DrawMaterial& drawMaterial)
	{

	}

	void DebugRenderer::LogDebugInfo()
	{

	}

}