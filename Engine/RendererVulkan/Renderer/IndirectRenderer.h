#pragma once

#include "Scene/RenderDataBuilder.h"

#include "RendererVulkan/RenderDevice/MeshPass.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Resource/DrawCall.h"

#include "Stl/SmartPtr.h"
#include "eastl/fixed_map.h"

namespace SG
{

#define SG_MAX_DRAW_CALL 100

	//! State machine, a functionality class to record draw command, and persist the render context.
	class IndirectRenderer
	{
	public:
		static void OnInit();
		static void OnShutdown();
		//! Collect the render data from RenderDataBuilder, make render resource and packed to drawcall.
		static void CollectRenderData(RefPtr<RenderDataBuilder> pRenderDataBuilder);

		static void Begin(VulkanCommandBuffer* pCmdBuf);
		static void End();

		static void Draw(EMeshPass meshPass);
	private:
		static void BindMesh(const DrawMesh& drawMesh);
		static void BindMaterial(const DrawMaterial& drawMaterial);
	private:
		static VulkanCommandBuffer* mpCmdBuf;

		static eastl::fixed_map<EMeshPass, vector<IndirectDrawCall>, (UInt32)EMeshPass::NUM_MESH_PASS> mDrawCallMap;
		static UInt32 mPackedVBCurrOffset;
		static UInt32 mPackedIBCurrOffset;

		static bool mbRendererInit;
		static bool mbBeginDraw;
		static bool mbDrawCallReady;
	};

}