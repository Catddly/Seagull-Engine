#pragma once

#include "RendererVulkan/Resource/CommonUBO.h"

#include "Stl/string.h"

namespace SG
{

	struct RenderMesh
	{
		string name;
		UInt64 vBSize = 0;
		UInt64 iBSize = 0;
		UInt64 vBOffset = 0;
		UInt64 iBOffset = 0;
		UInt32 meshBatchId = 0;

		PerMeshRenderData renderData = {};
	};

}