#pragma once

#include "Math/MathBasic.h"

namespace SG
{

	//! Equation of a plane is Ax + By + Cz + D = 0, where point(x, y, z) is on the plane.
	//! represent Normal(A, B, C) and Distance(D)
	//! So if a point is on the plane, this equation holds. -> (dot(UnknownPoint, normal) - distance == 0)
	class Plane
	{
	public:
		SG_CORE_API Plane() = default;
		SG_CORE_API Plane(const Vector4f& data);
		//! Construct from a point on the plane and its normal.
		SG_CORE_API Plane(const Vector3f& point, const Vector3f& normal);
		//Plane(Vector3f point1, Vector3f point2, Vector3f point3);

		SG_CORE_API bool IsPointOnPlane(const Vector3f& point);
		SG_CORE_API bool IsPointInFrontOfPlane(const Vector3f& point);

		SG_CORE_API float DistanceFromPlane(const Vector3f& point);

		Plane& operator/=(float value)
		{
			mData /= value;
			return *this;
		}

		operator Vector4f() const { return mData; }
	private:
		Vector4f mData= Vector4f(0.0f); // (x, y, z) is normal, and w is distance
	};

}