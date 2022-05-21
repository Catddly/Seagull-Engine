#pragma once

#include "Math/MathBasic.h"

namespace SG
{

	enum class ELoadOp
	{
		eLoad = 0,
		eClear,
		eDont_Care,
	};

	enum class EStoreOp
	{
		eStore = 0,
		eDont_Care,
	};

	struct LoadStoreClearOp
	{
		ELoadOp  loadOp;
		EStoreOp storeOp;
		ELoadOp  stencilLoadOp;
		EStoreOp stencilStoreOp;

		SG_INLINE bool operator==(const LoadStoreClearOp lhs) const
		{
			return (this->loadOp == lhs.loadOp) &&
				(this->storeOp == lhs.storeOp) &&
				(this->stencilLoadOp == lhs.stencilLoadOp) &&
				(this->stencilStoreOp == lhs.stencilStoreOp);
		}
	};

	// copy from the vulkan clear value definition
	union ClearColorValue
	{
		float  float32[4];
		Int32  int32[4];
		UInt32 uint32[4];
	};

	struct ClearDepthStencilValue
	{
		float  depth;
		UInt32 stencil;
	};

	union ClearValue
	{
		ClearColorValue color;
		ClearDepthStencilValue depthStencil;
	};

}