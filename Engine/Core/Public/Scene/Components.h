#pragma once

#include "Math/MathBasic.h"

namespace SG
{

	// temporary component system
	// should be replaced by ECS

	//! Pure data + function class.
	class TransformComponent
	{
	public:
		TransformComponent()
			:mPosition(0.0f, 0.0f, 0.0f), mScale(1.0f, 1.0f, 1.0f), mRotation(0.0f, 0.0f, 0.0f)
		{}

		SG_INLINE void SetPosition(const Vector3f& pos) { mPosition = pos; }
		SG_INLINE Vector3f GetPosition() const { return mPosition; };

		SG_INLINE void SetScale(const Vector3f& scale) { mScale = scale; }
		SG_INLINE Vector3f GetScale()    const { return mScale; };

		SG_INLINE void SetRotation(const Vector3f& rotation) { mRotation = rotation; }
		SG_INLINE Vector3f GetRotation() const { return mRotation; };

		SG_INLINE Matrix4f GetTransform() const
		{
			return glm::translate(Matrix4f(1.0f), mPosition) *
				glm::toMat4(Quternion(glm::radians(mRotation))) *
				glm::scale(Matrix4f(1.0f), mScale);
		}
	private:
		Vector3f mPosition;
		Vector3f mScale;
		Vector3f mRotation;
	};

}