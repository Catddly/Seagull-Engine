#include "StdAfx.h"
#include "Math/Frustum.h"

namespace SG
{

	Plane Frustum::GetFrontPlane() const
	{
		return mFrontPlane;
	}

	Plane Frustum::GetBackPlane() const
	{
		return mBackPlane;
	}

	Plane Frustum::GetRightPlane() const
	{
		return mRightPlane;
	}

	Plane Frustum::GetLeftPlane() const
	{
		return mLeftPlane;
	}

	Plane Frustum::GetTopPlane() const
	{
		return mTopPlane;
	}

	Plane Frustum::GetBottomPlane() const
	{
		return mBottomPlane;
	}

}