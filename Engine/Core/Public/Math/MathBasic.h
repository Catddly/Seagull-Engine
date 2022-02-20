#pragma once

#include "Defs/Defs.h"

#include "Stl/Hash.h"

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

	// column major matrix
	typedef Eigen::Matrix<Float32, 3, 3> Matrix3f;
	typedef Eigen::Matrix<Float32, 4, 4> Matrix4f;

	typedef Eigen::Matrix<Int32, 3, 3>   Matrix3i;
	typedef Eigen::Matrix<Int32, 4, 4>   Matrix4i;

#define PI 3.141592653589793238462643383279f

#define SG_ENGINE_UP_VEC()    Vector3f(0.0f, 1.0f, 0.0f) // Seagull engine is y-up right hand-side coordinate
#define SG_ENGINE_RIGHT_VEC() Vector3f(1.0f, 0.0f, 0.0f)

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

	SG_INLINE float Sin(float v)  { return ::sinf(v); }
	SG_INLINE float Cos(float v)  { return ::cosf(v); }
	SG_INLINE float Tan(float v)  { return ::tanf(v); }
	SG_INLINE float Abs(float v)  { return ::abs(v); }
	SG_INLINE int   Abs(int v)    { return ::abs(v); }
	SG_INLINE float Log2(float v) { return ::log2f(v); }
	SG_INLINE float Log10(float v) { return ::log10f(v); }
	SG_INLINE float Log(float v) { return ::log(v); }

	SG_INLINE bool   IsPowerOfTwo(UInt32 v) { return !(v & (v - 1)); }
	SG_INLINE UInt32 MinValueAlignTo(UInt32 v, UInt32 align) { return ((v + align - 1) / align) * align; }

	SG_INLINE float Clamp(float& v, float min, float max) { return v < min ? min : (v > max ? max : v); }

#ifdef SG_GRAPHICS_API_VULKAN
	//! Build a view matrix based on a view direction vector.
	//! @param [view] Point where the eyes on.
	//! @param [direction] Direction where the eyes look at.
	//! @param [up] Up vector of the world.
	//! @return a 4x4 matrix represent the view matrix.
	SG_INLINE Matrix4f BuildViewMatrixDirection(const Vector3f& view, const Vector3f& direction, const Vector3f& up)
	{
		const Vector3f z(direction.normalized());
		const Vector3f x(z.cross(up).normalized());
		const Vector3f y(x.cross(z));

		Matrix4f result = Matrix4f::Identity();
		result.col(0) << x(0), x(1), x(2), 0.0f;
		result.col(1) << y(0), y(1), y(2), 0.0f;
		result.col(2) << z(0), z(1), z(2), 0.0f;
		result.col(3) << -x.dot(view), -y.dot(view), -z.dot(view), 1.0f;

		return eastl::move(result);
	}

	//! Build a view matrix based on a position vector.
	//! @param [view] Point where the eyes on.
	//! @param [center] Center point where the eyes look at.
	//! @param [up] Up vector of the world.
	//! @return a 4x4 matrix represent the view matrix.
	SG_INLINE Matrix4f BuildViewMatrixCenter(const Vector3f& view, const Vector3f& center, const Vector3f& up)
	{
		return eastl::move(BuildViewMatrixDirection(view, view - center, up));
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

		float tanHalfFovy = Tan(fovYInRadians / 2.0f);

		Matrix4f result = Matrix4f::Zero();
		result(0, 0) = 1.0f / (aspect * tanHalfFovy);
		result(1, 1) = 1.0f / (tanHalfFovy);
		result(2, 2) = -(zFar + zNear) / (zFar - zNear);
		result(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
		result(3, 2) = -1.0f;
		return eastl::move(result);
	}

	SG_INLINE Matrix4f BuildOrthographicMatrix(float left, float right, float top, float bottom, float zNear, float zFar)
	{
		Matrix4f result = Matrix4f::Identity();
		result(0, 0) = 2.0f / (right - left);
		result(1, 1) = 2.0f / (top - bottom);
		result(2, 2) = -2.0f / (zFar - zNear);

		result(0, 3) = -(right + left) / (right - left);
		result(1, 3) = -(top + bottom) / (top - bottom);
		result(2, 3) = -(zFar + zNear) / (zFar - zNear);
		return eastl::move(result);
	}
#endif

}

// override hash functions
namespace eastl
{
	template <> 
	struct hash<SG::Vector3f>
	{
		size_t operator()(SG::Vector3f val) const 
		{ 
			size_t seed = eastl::hash<float>{}(val(0));
			seed ^= eastl::hash<float>{}(val(1));
			seed ^= eastl::hash<float>{}(val(2));
			return seed;
		}
	};
}
