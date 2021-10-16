#pragma once

#include "Defs/Defs.h"
#include "Vector.h"

#include <cmath>

namespace SG
{

#define PI 3.141592653589793238462643383279f

#define SG_ENGINE_UP_VEC() Vector3f(0.0f, 1.0f, 0.0f) // Seagull engine is y-up right hand-side coordinate

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

}