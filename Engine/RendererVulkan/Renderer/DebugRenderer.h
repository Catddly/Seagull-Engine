#pragma once

#include "RendererVulkan/Renderer/RenderInfo.h"
#include "RendererVulkan/RenderDevice/MeshPass.h"
#include "RendererVulkan/RenderDevice/DrawCall.h"

#include "Stl/vector.h"
#include "eastl/fixed_map.h"

namespace SG
{

	class VulkanContext;

	//! State machine, a functionality class to record draw command, and persist the render context.
	class DebugRenderer
	{
	public:
		static void OnInit(VulkanContext& context);
		static void OnShutdown();
		//! Collect the render data from RenderDataBuilder, make render resource and packed to drawcall.
		//static void CollectRenderData(RefPtr<RenderDataBuilder> pRenderDataBuilder);

		static void Begin(DrawInfo& drawInfo);
		static void End();

		static void Draw(EMeshPass meshPass);
	private:
		static void BindMesh(const DrawMesh& drawMesh);
		static void BindMaterial(const DrawMaterial& drawMaterial);

		static void LogDebugInfo();
	private:
		static VulkanContext* mpContext;
		static VulkanCommandBuffer* mpCmdBuf;
		static UInt32 mCurrFrameIndex;



		static bool mbRendererInit;
		static bool mbBeginDraw;
		static bool mbDrawCallReady;
	};

}