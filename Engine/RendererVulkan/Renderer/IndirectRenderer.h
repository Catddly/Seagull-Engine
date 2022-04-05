#pragma once

#include "Scene/RenderDataBuilder.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Backend/VulkanShader.h"
#include "RendererVulkan/Backend/VulkanPipelineSignature.h"
#include "RendererVulkan/Backend/VulkanPipeline.h"

#include "RendererVulkan/Resource/DrawCall.h"
#include "RendererVulkan/RenderDevice/MeshPass.h"

#include "Stl/SmartPtr.h"
#include "Stl/unordered_map.h"
#include "eastl/fixed_map.h"

namespace SG
{

#define SG_MAX_DRAW_CALL 100

	//! State machine, a functionality class to record draw command, and persist the render context.
	class IndirectRenderer
	{
	public:
		static void OnInit(VulkanContext& context);
		static void OnShutdown();
		//! Collect the render data from RenderDataBuilder, make render resource and packed to drawcall.
		static void CollectRenderData(RefPtr<RenderDataBuilder> pRenderDataBuilder);

		static void Begin(VulkanCommandBuffer* pCmdBuf);
		static void End();

		static void DoCulling();
		static void Draw(EMeshPass meshPass);
	private:
		static void BindMesh(const DrawMesh& drawMesh);
		static void BindMaterial(const DrawMaterial& drawMaterial);
	private:
		static VulkanContext* mpContext;
		static VulkanCommandBuffer* mpCmdBuf;

		static eastl::fixed_map<EMeshPass, vector<IndirectDrawCall>, (UInt32)EMeshPass::NUM_MESH_PASS> mDrawCallMap;
		static UInt32 mPackedVBCurrOffset;
		static UInt32 mPackedVIBCurrOffset;
		static UInt32 mPackedIBCurrOffset;
		static UInt32 mCurrDrawCallIndex;

		// resource to do gpu culling and record indirect draw commands.
		static RefPtr<VulkanPipelineSignature> mpGPUCullingPipelineSignature;
		static VulkanPipeline*                 mpGPUCullingPipeline;
		static RefPtr<VulkanShader>            mpGPUCullingShader;

		static RefPtr<VulkanPipelineSignature> mpDrawCallCompactPipelineSignature;
		static VulkanPipeline*                 mpDrawCallCompactPipeline;
		static RefPtr<VulkanShader>            mpDrawCallCompactShader;

		static bool mbRendererInit;
		static bool mbBeginDraw;
		static bool mbDrawCallReady;

	};

}