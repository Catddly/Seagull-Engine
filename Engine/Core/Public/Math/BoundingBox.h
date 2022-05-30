#pragma once

#include "Math/MathBasic.h"

namespace SG
{

	struct BoundingBox
	{
		Vector3f minBound;
		Vector3f maxBound;
	};

	SG_INLINE void BBoxReset(BoundingBox& bbox)
	{
		bbox.minBound = { SG_MAX_FLOAT_VALUE, SG_MAX_FLOAT_VALUE, SG_MAX_FLOAT_VALUE };
		bbox.maxBound = { SG_MIN_FLOAT_VALUE, SG_MIN_FLOAT_VALUE, SG_MIN_FLOAT_VALUE };
	}

	SG_INLINE void BBoxMerge(BoundingBox& bbox, const Vector3f& mergePoint)
	{
		Floor(bbox.minBound, mergePoint);
		Ceil(bbox.maxBound, mergePoint);
	}

	SG_INLINE void BBoxMerge(BoundingBox& bbox, const BoundingBox& mergeBox)
	{
		Floor(bbox.minBound, mergeBox.minBound);
		Ceil(bbox.maxBound, mergeBox.maxBound);
	}

	SG_INLINE Vector3f BBoxCenter(const BoundingBox& bbox)
	{
		Vector3f center;
		center.x = { (bbox.minBound.x + bbox.maxBound.x) * 0.5f };
		center.y = { (bbox.minBound.y + bbox.maxBound.y) * 0.5f };
		center.z = { (bbox.minBound.z + bbox.maxBound.z) * 0.5f };
		return eastl::move(center);
	}

	SG_INLINE Vector3f BBoxExtent(const BoundingBox& bbox)
	{
		Vector3f extent;
		extent.x = { (bbox.maxBound.x - bbox.minBound.x) * 0.5f };
		extent.y = { (bbox.maxBound.y - bbox.minBound.y) * 0.5f };
		extent.z = { (bbox.maxBound.z - bbox.minBound.z) * 0.5f };
		return eastl::move(extent);
	}

}