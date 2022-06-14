#pragma once

#include "Math/MathBasic.h"

#include "Profile/Profile.h"

namespace SG
{

	struct AABB
	{
		Vector3f min;
		Vector3f max;

		static constexpr float MIN = SG_MIN_FLOAT_VALUE;
		static constexpr float MAX = SG_MAX_FLOAT_VALUE;
	};

	// serialization code
	void to_json(SG::json& node, const AABB& v);
	void from_json(const SG::json& node, AABB& v);

	SG_INLINE void AABBReset(AABB& bbox)
	{
		SG_PROFILE_FUNCTION();

		bbox.min = { AABB::MAX, AABB::MAX, AABB::MAX };
		bbox.max = { AABB::MIN, AABB::MIN, AABB::MIN };
	}

	SG_INLINE void AABBMerge(AABB& bbox, const Vector3f& mergePoint)
	{
		SG_PROFILE_FUNCTION();

		Floor(bbox.min, mergePoint);
		Ceil(bbox.max, mergePoint);
	}

	SG_INLINE void AABBMerge(AABB& bbox, const AABB& mergeBox)
	{
		SG_PROFILE_FUNCTION();

		Floor(bbox.min, mergeBox.min);
		Ceil(bbox.max, mergeBox.max);
	}

	SG_INLINE Vector3f AABBCenter(const AABB& bbox)
	{
		SG_PROFILE_FUNCTION();

		Vector3f center;
		center.x = { (bbox.min.x + bbox.max.x) * 0.5f };
		center.y = { (bbox.min.y + bbox.max.y) * 0.5f };
		center.z = { (bbox.min.z + bbox.max.z) * 0.5f };
		return eastl::move(center);
	}

	SG_INLINE Vector3f AABBExtent(const AABB& bbox)
	{
		SG_PROFILE_FUNCTION();

		Vector3f extent;
		extent.x = { (bbox.max.x - bbox.min.x) * 0.5f };
		extent.y = { (bbox.max.y - bbox.min.y) * 0.5f };
		extent.z = { (bbox.max.z - bbox.min.z) * 0.5f };
		return eastl::move(extent);
	}

	SG_INLINE AABB AABBTransform(const AABB& bbox, const Matrix4f& transform)
	{
		SG_PROFILE_FUNCTION();

		Vector3f corners[8] = {
			bbox.min,
			{ bbox.min.x, bbox.min.y, bbox.max.z },
			{ bbox.min.x, bbox.max.y, bbox.min.z },
			{ bbox.max.x, bbox.min.y, bbox.min.z },
			{ bbox.min.x, bbox.max.y, bbox.max.z },
			{ bbox.max.x, bbox.min.y, bbox.max.z },
			{ bbox.max.x, bbox.max.y, bbox.min.z },
			bbox.max
		};

		AABB res;
		AABBReset(res);

		for (int i = 0; i < 8; ++i)
		{
			const Vector3f transformed = Vector3f(transform * Vector4f(corners[i], 1.0f));
			AABBMerge(res, transformed);
		}
		return eastl::move(res);
	}

}