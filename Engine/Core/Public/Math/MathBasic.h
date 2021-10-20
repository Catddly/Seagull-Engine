#pragma once

#include "Defs/Defs.h"

#include <eigen/Dense>

#include <cmath>

namespace SG
{

	typedef Eigen::Vector2f Vector2f;
	typedef Eigen::Vector3f Vector3f;
	typedef Eigen::Vector4f Vector4f;

	typedef Eigen::Vector2i Vector2i;
	typedef Eigen::Vector3i Vector3i;
	typedef Eigen::Vector4i Vector4i;

	typedef Eigen::Matrix<Float32, 3, 3> Matrix3f;
	typedef Eigen::Matrix<Float32, 4, 4> Matrix4f;

	typedef Eigen::Matrix<Int32, 3, 3>   Matrix3i;
	typedef Eigen::Matrix<Int32, 4, 4>   Matrix4i;

#define PI 3.141592653589793238462643383279f

#define SG_ENGINE_UP_VEC() Vector3f(0.0f, 1.0f, 0.0f) // Seagull engine is y-up right hand-side coordinate

	// Seagull Engine Take Left-Hand Y-Up Coordinate System
	// 
	//            y
	//            ^
	//            |       z
	//            |     -
	//            |   -
	//            | -
	//            0--------->  x
	//

	SG_INLINE float DegreesToRadians(float degrees)
	{
		return degrees * PI / 180.0f;
	}

	SG_INLINE float RadiansToDegrees(float radians)
	{
		return radians * 180.0f / PI;
	}

	SG_INLINE float Sin(float v) { return ::sinf(v); }
	SG_INLINE float Cos(float v) { return ::cosf(v); }
	SG_INLINE float Tan(float v) { return ::tanf(v); }
	SG_INLINE float Abs(float v) { return ::abs(v); }
	SG_INLINE int   Abs(int v)   { return ::abs(v); }

#ifdef SG_GRAPHICS_API_VULKAN
	//! Build a view matrix.
	//! @param [view] Point where the eyes on.
	//! @param [center] Point where the eyes look at.
	//! @param [up] Up vector of the world.
	//! @return a 4x4 matrix represent the view matrix.
	SG_INLINE Matrix4f BuildViewMatrix(const Vector3f& view, const Vector3f& center, const Vector3f& up)
	{
		const Vector3f front((view - center).normalized());
		const Vector3f right(front.cross(up).normalized());
		const Vector3f u(right.cross(front));

		Matrix4f res = Matrix4f::Identity();
		res.col(0) << right(0), right(1), right(2), 0.0f;
		res.col(1) << u(0), -u(1), u(2), 0.0f;
		res.col(2) << front(0), front(1), front(2), 0.0f;
		res.col(3) << -(right.dot(view)), -(u.dot(view)), -(front.dot(view)), 1.0f;
		return eastl::move(res);
	}

	//! Build a perspective matrix.
	//! @param [fovInRadians] Field of view in radians, an angle present the view cone.
	//! @param [aspect] Aspect describe the camera's view in 2D. To be width / height.
	//! @param [zNear] near z plane value.
	//! @param [zFar] far z plane value.
	//! @return a 4x4 matrix represent the perspective matrix.
	SG_INLINE Matrix4f BuildPerspectiveMatrix(float fovYInRadians, float aspect, float zNear, float zFar)
	{
		// the aspect should not be zero
		SG_ASSERT(Abs(aspect - SG_FLOAT_EPSILON) > 0.0f);

		float focal_length = 1.0f / Tan(fovYInRadians / 2.0f);

		float x = focal_length / aspect;
		float y = -focal_length;
		float A = zNear / (zFar - zNear);
		float B = zFar * A;

		Matrix4f result = Matrix4f::Zero();
		result(0, 0) = x;
		result(1, 1) = y;
		result(2, 2) = A;
		result(2, 3) = B;
		result(3, 2) = -1.0f;

		return eastl::move(result);
	}

	SG_INLINE Matrix4f BuildOrthographicMatrix(float left, float right, float top, float bottom, float zNear, float zFar)
	{
		Matrix4f result = Matrix4f::Identity();
		result(0, 0) = 2.0f / (right - left);
		result(1, 1) = -2.0f / (top - bottom);
		result(2, 2) = -1.0f / (zFar - zNear);
		result(0, 3) = -(right + left) / (right - left);
		result(1, 3) = -(top + bottom) / (top - bottom);
		result(2, 3) = zNear / (zFar - zNear);

		return eastl::move(result);
	}
#endif

}