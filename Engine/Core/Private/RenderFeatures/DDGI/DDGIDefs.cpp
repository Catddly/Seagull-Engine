#include "StdAfx.h"
#include "RenderFeatures/DDGI/DDGIDefs.h"

namespace SG
{

	Vector3f CalcProbeSpacing(const AABB& volumn)
	{
		const Vector3f extent = AABBExtent(volumn);

		const float NUM_PROBE_X = DDGIVolumnComponent::DDGI_NUM_PROBE_X + 2 - 1;
		const float NUM_PROBE_Y = DDGIVolumnComponent::DDGI_NUM_PROBE_Y + 2 - 1;
		const float NUM_PROBE_Z = DDGIVolumnComponent::DDGI_NUM_PROBE_Z + 2 - 1;

		return extent / Vector3f{ NUM_PROBE_X, NUM_PROBE_Y, NUM_PROBE_Z };
	}

}