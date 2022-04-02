#pragma once

#include "Math/Plane.h"

#include "Stl/vector.h"

namespace SG
{

	class Frustum
	{
	public:
		SG_CORE_API Frustum() = default;
		SG_CORE_API Frustum(const Plane& front, const Plane& back, const Plane& right, const Plane& left, const Plane& top, const Plane& bottom)
			:mFrontPlane(front), mBackPlane(back), mRightPlane(right), mLeftPlane(left), mTopPlane(top), mBottomPlane(bottom)
		{}

		SG_CORE_API Plane GetFrontPlane() const;
		SG_CORE_API Plane GetBackPlane() const;
		SG_CORE_API Plane GetRightPlane() const;
		SG_CORE_API Plane GetLeftPlane() const;
		SG_CORE_API Plane GetTopPlane() const;
		SG_CORE_API Plane GetBottomPlane() const;
	private:
		Plane mFrontPlane;
		Plane mBackPlane;
		Plane mRightPlane;
		Plane mLeftPlane;
		Plane mTopPlane;
		Plane mBottomPlane;
	};

}