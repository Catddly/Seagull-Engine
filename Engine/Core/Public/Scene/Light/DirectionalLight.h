#pragma once

#include "Math/MathBasic.h"

namespace SG
{

	class DirectionalLight
	{
	public:
		explicit DirectionalLight(const Vector3f postion, const Vector3f& direction, const Vector3f& color)
			:mPosition(postion), mDirection(direction), mColor(color)
		{}
		~DirectionalLight() = default;

		SG_INLINE void     SetPosition(const Vector3f& position) { mPosition = position; mIsDirty = true; }
		SG_INLINE Vector3f GetPosition() const { return mPosition; }

		SG_INLINE void     SetDirection(const Vector3f& direction) { mDirection = direction; mIsDirty = true; }
		SG_INLINE Vector3f GetDirection() const { return mDirection; }

		SG_INLINE void     SetColor(const Vector3f& color) { mColor = color; mIsDirty = true; }
		SG_INLINE Vector3f GetColor() const { return mColor; }

		SG_INLINE Matrix4f GetViewProj() const
		{
			return BuildPerspectiveMatrix(glm::radians(45.0f), 1.0f, 1.0f, 96.0f) *
				BuildViewMatrixCenter(mPosition, { 0.0f, 0.0f, 0.0f }, SG_ENGINE_UP_VEC());
		}
	private:
		Vector3f mPosition;
		Vector3f mDirection;
		Vector3f mColor;
		mutable bool mIsDirty = true;
	};

}