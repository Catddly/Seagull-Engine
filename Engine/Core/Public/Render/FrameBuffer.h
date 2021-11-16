#pragma once

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

	struct LoadStoreClearOP
	{
		ELoadOp  loadOp;
		EStoreOp storeOp;
		ELoadOp  stencilLoadOp;
		EStoreOp stencilStoreOp;
	};

}