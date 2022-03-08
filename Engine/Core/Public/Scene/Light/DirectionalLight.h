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

		void     SetPosition(const Vector3f& position) { mPosition = position; mIsDirty = true; }
		Vector3f GetPosition() const { return mPosition; }

		void     SetDirection(const Vector3f& direction) { mDirection = direction; mIsDirty = true; }
		Vector3f GetDirection() const { return mDirection; }

		void     SetColor(const Vector3f& color) { mColor = color; mIsDirty = true; }
		Vector3f GetColor() const { return mColor; }
	private:
		Vector3f mPosition;
		Vector3f mDirection;
		Vector3f mColor;
		mutable bool mIsDirty = true;
	};

}