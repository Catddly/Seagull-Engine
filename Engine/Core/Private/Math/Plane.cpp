#include "StdAfx.h"
#include "Math/Plane.h"

namespace SG
{

	Plane::Plane(const Vector4f& data)
		:mData(data)
	{
	}

	Plane::Plane(const Vector3f& point, const Vector3f& normal)
	{
		mData.x = normal.x;
		mData.y = normal.y;
		mData.z = normal.z;
		mData.w = -glm::dot(point, Vector3f(mData));
	}

	bool Plane::IsPointOnPlane(const Vector3f& point)
	{
		return DistanceFromPlane(point) == 0.0f;
	}

	bool Plane::IsPointInFrontOfPlane(const Vector3f& point)
	{
		return DistanceFromPlane(point) > 0.0f;
	}

	float Plane::DistanceFromPlane(const Vector3f& point)
	{
		return glm::dot(Vector3f(mData), point) - mData.w;
	}

}