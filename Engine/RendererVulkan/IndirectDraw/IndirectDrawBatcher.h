#pragma once

#include "Base/BasicTypes.h"

#include "RendererVulkan/Resource/DrawCall.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"

#include "Stl/vector.h"
#include "eastl/array.h"
#include "eastl/fixed_map.h"

namespace SG
{

#define SG_MAX_DRAW_CALL  100

	enum class EMeshPass : UInt32
	{
		eForward = 0,
		eForwardInstanced,
		NUM_MESH_PASS,
		eUndefined,
	};

	struct IndirectDrawCall
	{
		UInt32 indirectDrawCount;
		UInt32 offset;
		UInt32 stride;
	};

	class IndirectDrawBatcher
	{
	public:
		void CreateIndirectBuffer();
		void DestroyIndirectBuffer();

		void AddMeshPassDrawCall(EMeshPass meshPass, const DrawCall& drawCall);
		void FinishBuildMeshPass(VulkanBuffer* pIndirectCmdBuffer);

		void Draw(EMeshPass meshPass, VulkanCommandBuffer& buf);
	private:
		typedef vector<DrawIndexedIndirectCommand> IndirectDrawCommands;
		eastl::fixed_map<EMeshPass, IndirectDrawCommands, (UInt32)EMeshPass::NUM_MESH_PASS> mIndirectBatchesMap;
		eastl::array<IndirectDrawCall, (UInt32)EMeshPass::NUM_MESH_PASS> mIndirectDrawCalls;

		bool mbIndirectDrawReady = false;
	};

}