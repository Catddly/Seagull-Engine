#pragma once

#include "Defs/Defs.h"
#include "Base/BasicTypes.h"

namespace SG
{

	enum class EPipelineStageQueryType : UInt32
	{
		efInput_Assembly_Vertices = BIT(0),
		efInput_Assembly_Primitives = BIT(1),
		efVertex_Shader_Invocations = BIT(2),
		efGeometry_Shader_Invocation = BIT(3),
		efGeometry_Shader_Primitives = BIT(4),
		efClipping_Invocations = BIT(5),
		efClipping_Primitives = BIT(6),
		efFragment_Shader_Invocations = BIT(7),
		efTessellation_Control_Shader_Patches = BIT(8),
		efTessellation_Evaluation_Shader_Invocations = BIT(9),
		efCompute_Shader_Invocations = BIT(10),
	};
	SG_ENUM_CLASS_FLAG(UInt32, EPipelineStageQueryType);

	enum class ERenderQueryType
	{
		ePipeline_Statistics = 0,
		eTimeStamp,
	};

	struct QueryResult
	{
		UInt64 result;
	};

}