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

	// Seagull Engine Take Right-Hand Y-Up Coordinate System
	// 
	//            y
	//            ^
	//            |
	//            |
	//            0--------->  x
	//           -
	//          -
	//         -
	//        z
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
		const Vector3f front((center - view).normalized());
		const Vector3f right(front.cross(up).normalized());
		const Vector3f u(right.cross(front));

		Matrix4f res = Matrix4f::Identity();
		res.col(0) << right(0), right(1), right(2), 0.0f;
		res.col(1) << u(0), u(1), u(2), 0.0f;
		res.col(2) << -front(0), -front(1), -front(2), 0.0f;
		res.col(3) << -(right.dot(view)), -(u.dot(view)), front.dot(view), 1.0f;
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

		const float TAN_HALF_FOVY = Tan(fovYInRadians / 2.0f);

		Matrix4f result = Matrix4f::Zero();
		result(0, 0) = 1.0f / (aspect * TAN_HALF_FOVY);
		result(1, 1) = 1.0f / (TAN_HALF_FOVY);
		result(2, 2) = zFar / (zNear - zFar); // 0 to 1 z Clip space
		result(2, 3) = -1.0f;
		result(3, 2) = -(zFar * zNear) / (zFar - zNear);

		return eastl::move(result);
	}
#endif

}