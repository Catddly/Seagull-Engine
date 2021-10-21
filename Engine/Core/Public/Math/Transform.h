#pragma once

#include <eigen/Geometry>

#include "Math/Matrix.h"
#include "Math/Vector.h"

namespace SG
{

	typedef Eigen::Transform<float, 3, Eigen::TransformTraits::Affine> Transform;

	//! Represent rotation alone an axis with a fixed speed.
	typedef Eigen::AngleAxisf AngleAxis;
	//! Represent translation with a fixed speed.
	typedef Eigen::Translation<float, 3> Translation;
	//! Represent scale operation with a fixed speed.
	typedef Eigen::UniformScaling<float> Scaler;

	SG_INLINE void Translate(Matrix4f& matrix, const Vector3f& pos)
	{
		matrix(0, 3) += pos(0);
		matrix(1, 3) += pos(1);
		matrix(2, 3) += pos(2);
	}

	SG_INLINE void TranslateTo(Matrix4f& matrix, const Vector3f& pos)
	{
		matrix.col(3) << pos(0), pos(1), pos(2), 1.0f;
	}

	SG_INLINE void TranslateToX(Matrix4f& matrix, float x)
	{
		matrix(0, 3) = x;
	}

	SG_INLINE void TranslateToY(Matrix4f& matrix, float y)
	{
		matrix(1, 3) = y;
	}

	SG_INLINE void TranslateToZ(Matrix4f& matrix, float z)
	{
		matrix(2, 3) = z;
	}

	SG_INLINE void Scale(Matrix4f& matrix, const Vector3f& scale)
	{
		matrix(0, 0) *= scale(0);
		matrix(1, 1) *= scale(1);
		matrix(2, 2) *= scale(2);
	}

	SG_INLINE void ScaleTo(Matrix4f& matrix, const Vector3f& scale)
	{
		matrix(0, 0) = scale(0);
		matrix(1, 1) = scale(1);
		matrix(2, 2) = scale(2);
	}

	SG_INLINE void Rotate(Matrix4f& matrix, const Vector3f& axis, float degrees)
	{
		Transform t(AngleAxis(DegreesToRadians(degrees), axis));
		matrix *= t.matrix();
	}

	SG_INLINE void RotateTo(Matrix4f& matrix, const Vector3f& rotation)
	{
		Transform t(AngleAxis(DegreesToRadians(rotation(0)), Vector3f{ 1.0f, 0.0f, 0.0f }));
		t *= AngleAxis(DegreesToRadians(rotation(1)), Vector3f{ 0.0f, 1.0f, 0.0f }) * 
			AngleAxis(DegreesToRadians(rotation(2)), Vector3f{ 0.0f, 0.0f, 1.0f });
		TranslateTo(t.matrix(), { matrix(0,3), matrix(1,3), matrix(2,3) });
		ScaleTo(t.matrix(), { matrix(0,0), matrix(1,1), matrix(2,2) });
		matrix = t.matrix();
	}

	SG_INLINE Matrix4f BuildTransformMatrix(const Vector3f& position, const Vector3f& scale, const Vector3f& rotation)
	{
		Transform t = Translation(position) * 
			AngleAxis(DegreesToRadians(rotation(0)), Vector3f{ 1.0f, 0.0f, 0.0f }) * 
			AngleAxis(DegreesToRadians(rotation(1)), Vector3f{ 0.0f, 1.0f, 0.0f }) * 
			AngleAxis(DegreesToRadians(rotation(2)), Vector3f{ 0.0f, 0.0f, 1.0f });
		ScaleTo(t.matrix(), scale);
		return eastl::move(t.matrix());
	}

	SG_INLINE Matrix4f BuildTransformMatrix(const Vector3f& position, float scale, const Vector3f& rotation)
	{
		Transform t = Translation(position) *
			AngleAxis(DegreesToRadians(rotation(0)), Vector3f{ 1.0f, 0.0f, 0.0f }) *
			AngleAxis(DegreesToRadians(rotation(1)), Vector3f{ 0.0f, 1.0f, 0.0f }) *
			AngleAxis(DegreesToRadians(rotation(2)), Vector3f{ 0.0f, 0.0f, 1.0f }) *
			Scaler(scale);
		return eastl::move(t.matrix());
	}

	SG_INLINE Vector3f PitchYawToUnitVector(const Vector3f& rotation)
	{
		Vector3f unitVec;
		const float rx = DegreesToRadians(rotation(0));
		const float ry = DegreesToRadians(rotation(1));

		unitVec(0) = Sin(ry);
		unitVec(1) = Sin(rx);
		unitVec(2) = Cos(ry) * Cos(rx);

		return eastl::move(unitVec.normalized());
	}

}