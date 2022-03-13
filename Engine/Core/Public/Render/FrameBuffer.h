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

	struct ClearValue
	{
		Vector4f color = { 0.0f, 0.0f, 0.0f, 1.0f };
		struct
		{
			float   depth = 1.0f;
			UInt32  stencil = 0;
		} depthStencil;
	};

}