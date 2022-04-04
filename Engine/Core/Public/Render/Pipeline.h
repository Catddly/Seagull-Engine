#pragma once

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

namespace SG
{

	enum class EPipelineType
	{
		eGraphic = 0,
		eCompute,
		eTransfer,
		MAX_COUNT,
	};

	enum class EVertexInputRate
	{
		ePerVertex = 0,
		ePerInstance
	};

	enum class ECullMode
	{
		eBack = 0,
		eFront,
		eFrontAndBack,
		eNone,
	};

	enum class EPolygonMode
	{
		eFill = 0,
		eLine,
		ePoint,
	};

	enum class EPipelineStage : UInt32
	{
		efIndirect_Read = BIT(0),
		efIndex_Read = BIT(1),
		efVertex_Read = BIT(2),
		efUniforn_Read = BIT(3),
		efInput_Attachment_Read = BIT(4),
		efShader_Read = BIT(5),
		efShader_Write = BIT(6),
		efColor_Attachment_Read = BIT(7),
		efColor_Attachment_Write = BIT(8),
		efDepth_Stencil_Attachment_Read = BIT(9),
		efDepth_Stencil_Attachment_Write = BIT(10),
		efTransfer_Read = BIT(11),
		efTransfer_Write = BIT(12),
		efHost_Read = BIT(13),
		efHost_Write = BIT(14),
		efMemory_Read = BIT(15),
		efMemory_Write = BIT(16),
	};
	SG_ENUM_CLASS_FLAG(UInt32, EPipelineStage);

	interface Pipeline
	{
		virtual ~Pipeline() = default;
	};

}