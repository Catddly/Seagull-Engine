#pragma once

#include "Math/MathBasic.h"
#include "Math/BoundingBox.h"

namespace SG
{

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// DDGIVolumnComponent
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct DDGIVolumnComponent
	{
		// for now, use fixed size probes
		static constexpr UInt32 DDGI_NUM_PROBE_X = 8;
		static constexpr UInt32 DDGI_NUM_PROBE_Y = 8;
		static constexpr UInt32 DDGI_NUM_PROBE_Z = 8;
		static constexpr UInt32 DDGI_NUM_PROBES = DDGI_NUM_PROBE_X * DDGI_NUM_PROBE_Y * DDGI_NUM_PROBE_Z;

		AABB     volumn;
		Vector3f probeSpacing;

		DDGIVolumnComponent() = default;
	};

	Vector3f CalcProbeSpacing(const AABB& volumn);

}