#pragma once

#include "Scene/RenderDataBuilder.h"

#include "RendererVulkan/RenderDevice/MeshPass.h"
#include "RendererVulkan/RenderDevice/DrawCall.h"
#include "RendererVulkan/Backend/VulkanCommand.h"

#include "Stl/SmartPtr.h"
#include "eastl/fixed_map.h"

namespace SG
{

	//! State machine, a functionality class to record draw command,
	//! and persist the render context.
	class ForwardDP
	{
	public:
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

		static eastl::fixed_map<EMeshPass, vector<DrawCall>, (UInt32)EMeshPass::NUM_MESH_PASS> mDrawCallMap;
		static UInt64 mPackedVBCurrOffset;
		static UInt64 mPackedVIBCurrOffset;
		static UInt64 mPackedIBCurrOffset;

		static bool mbBeginDraw;
		static bool mbDrawCallReady;
	};

}