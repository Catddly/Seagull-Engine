#pragma once

#include "Scene/RenderDataBuilder.h"

#include "RendererVulkan/RenderDevice/MeshPass.h"
#include "RendererVulkan/Backend/VulkanCommand.h"
#include "RendererVulkan/Resource/DrawCall.h"

#include "Stl/SmartPtr.h"
#include "eastl/fixed_map.h"

namespace SG
{

	//! State machine, a functionality class to record draw command, and persist the render context.
	class IndirectRenderer
	{
	public:
		//! Collect the render data from RenderDataBuilder, make render resource and packed to drawcall.
		static void CollectRenderData(RefPtr<RenderDataBuilder> pRenderDataBuilder);

		static void Begin(VulkanCommandBuffer* pCmdBuf);
		static void End();

		static void Draw(EMeshPass meshPass);
	private:
		static void BindMesh(const DrawMesh& drawMesh);
		static void BindInstanceMesh(const DrawMesh& drawMesh);
		static void BindMaterial(const DrawMaterial& drawMaterial);
	private:
		static VulkanCommandBuffer* mpCmdBuf;

		static eastl::fixed_map<EMeshPass, vector<DrawIndexedIndirectCommand>, (UInt32)EMeshPass::NUM_MESH_PASS> mDrawCallMap;
		static eastl::fixed_map<EMeshPass, IndirectDrawCall, (UInt32)EMeshPass::NUM_MESH_PASS> mIndirectDrawCallMap;
		static UInt32 mPackedVBCurrOffset;
		static UInt32 mPackedIBCurrOffset;

		static bool mbBeginDraw;
		static bool mbDrawCallReady;
	};

}