#pragma once

#include "Defs/Defs.h"

#include "Stl/Hash.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <cmath>

namespace SG
{

	typedef glm::vec2 Vector2f;
	typedef glm::vec3 Vector3f;
	typedef glm::vec4 Vector4f;

	typedef glm::ivec2 Vector2i;
	typedef glm::ivec3 Vector3i;
	typedef glm::ivec4 Vector4i;

	typedef glm::mat3 Matrix3f;
	typedef glm::mat4 Matrix4f;

	typedef glm::mat<3, 3, int, glm::defaultp> Matrix3i;
	typedef glm::mat<4, 4, int, glm::defaultp> Matrix4i;

#define PI 3.141592653589793238462643383279f

#define SG_ENGINE_UP_VEC()    Vector3f( 0.0f, 1.0f, 0.0f) // Seagull engine is y-up right hand-side coordinate
#define SG_ENGINE_RIGHT_VEC() Vector3f(-1.0f, 0.0f, 0.0f)

	// Seagull Engine Take Right-Handed Y-Up Coordinate System
	// 
	//            y
	//            ^
	//            |       z
	//            |     -
	//            |   -
	//            | -
	// x <--------0
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
	
	//SG_INLINE Matrix4f BuildViewMatrixDirection(const Vector3f& view, const Vector3f& direction, const Vector3f& up)
	//{
	//	const Vector3f z(direction.normalized());
	//	const Vector3f x(z.cross(up).normalized());
	//	const Vector3f y(x.cross(z));

	//	Matrix4f result = Matrix4f::Identity();
	//	result.col(0) << x(0), x(1), x(2), 0.0f;
	//	result.col(1) << y(0), y(1), y(2), 0.0f;
	//	result.col(2) << z(0), z(1), z(2), 0.0f;
	//	result.col(3) << -x.dot(view), -y.dot(view), -z.dot(view), 1.0f;

	//	return eastl::move(result);
	//}

	//! Build a view matrix based on a position vector.
	//! @param [view] Point where the eyes on.
	//! @param [center] Center point where the eyes look at.
	//! @param [up] Up vector of the world.
	//! @return a 4x4 matrix represent the view matrix.
	//! 
	SG_INLINE Matrix4f BuildViewMatrixCenter(const Vector3f& view, const Vector3f& center, const Vector3f& up)
	{
		return glm::lookAt(view, center, up);
	}

	//! Build a perspective matrix.
	//! @param [fovInRadians] Field of view in radians, an angle present the view cone.
	//! @param [aspect] Aspect describe the camera's view in 2D. To be width / height.
	//! @param [zNear] near z plane value.
	//! @param [zFar] far z plane value.
	//! @return a 4x4 matrix represent the perspective matrix.
	SG_INLINE Matrix4f BuildPerspectiveMatrix(float fovYInRadians, float aspect, float zNear, float zFar)
	{
		return glm::perspective(fovYInRadians, aspect, zNear, zFar);
	}

	SG_INLINE Matrix4f BuildOrthographicMatrix(float left, float right, float top, float bottom, float zNear, float zFar)
	{
		return glm::ortho(left, right, bottom, top, zNear, zFar);
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
			size_t seed = eastl::hash<float>{}(val.x);
			seed ^= eastl::hash<float>{}(val.y);
			seed ^= eastl::hash<float>{}(val.z);
			return seed;
		}
	};
}
