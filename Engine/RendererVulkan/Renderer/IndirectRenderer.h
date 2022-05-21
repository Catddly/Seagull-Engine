#pragma once

#include "Scene/RenderDataBuilder.h"

#include "RendererVulkan/Renderer/RenderInfo.h"
#include "RendererVulkan/RenderDevice/MeshPass.h"
#include "RendererVulkan/RenderDevice/DrawCall.h"

#include "Stl/SmartPtr.h"
#include "Stl/unordered_map.h"
#include "eastl/fixed_map.h"

namespace SG
{

#define SG_MAX_DRAW_CALL 100

	class VulkanContext;
	class VulkanCommandBuffer;
	class VulkanSemaphore;
	class VulkanPipeline;
	class VulkanFence;
	class VulkanPipelineSignature;
	class VulkanShader;

	//! State machine, a functionality class to record draw command, and persist the render context.
	class IndirectRenderer
	{
	public:
		static void OnInit(VulkanContext& context);
		static void OnShutdown();
		//! Collect the render data from RenderDataBuilder, make render resource and packed to drawcall.
		static void CollectRenderData(RefPtr<RenderDataBuilder> pRenderDataBuilder);

		static void Begin(DrawInfo& drawInfo);
		static void End();

		static void CullingReset();
		static void DoCulling();
		static void CopyStatisticsData();
		static void WaitForStatisticsCopyed();

		static void Draw(EMeshPass meshPass);
		// temp
		static void DrawWithoutBindMaterial(EMeshPass meshPass);
	private:
		static void BindMesh(const DrawMesh& drawMesh);
		static void BindMaterial(const DrawMaterial& drawMaterial);

		static void LogDebugInfo();
	private:
		static VulkanContext* mpContext;
		static VulkanCommandBuffer* mpCmdBuf;
		static UInt32 mCurrFrameIndex;

		static eastl::fixed_map<EMeshPass, vector<IndirectDrawCall>, (UInt32)EMeshPass::NUM_MESH_PASS> mDrawCallMap;
		static UInt32 mPackedVBCurrOffset;
		static UInt32 mPackedVIBCurrOffset;
		static UInt32 mPackedIBCurrOffset;
		static UInt32 mCurrDrawCallIndex;

		static vector<VulkanCommandBuffer> mResetCommands;
		static vector<VulkanCommandBuffer> mCullingCommands;
		static vector<VulkanCommandBuffer> mTransferCommands;
		static vector<VulkanFence*> mTransferFences;
		static VulkanSemaphore* mpWaitResetSemaphore;

		static RefPtr<VulkanPipelineSignature> mpResetCullingPipelineSignature;
		static VulkanPipeline*                 mpResetCullingPipeline;
		static RefPtr<VulkanShader>            mpResetCullingShader;

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